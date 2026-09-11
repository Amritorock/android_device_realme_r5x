/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "android.hardware.biometrics.fingerprint-service.r5x"

#include "RbsDevice.h"

#include <android-base/strings.h>
#include <cerrno>
#include <dlfcn.h>
#include <limits.h>
#include <log/log.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <string>

using ::android::base::StartsWith;

namespace aidl::android::hardware::biometrics::fingerprint {

namespace {
constexpr char kRbsLib[] = "libRbsFlow.so";
constexpr uint32_t kNoGid = 9999;
constexpr uint32_t kMaxFingers = 5;
}  // namespace

RbsDevice* RbsDevice::sInstance = nullptr;

bool RbsDevice::open() {
    sInstance = this;

    ALOGD("Opening fingerprint hal library...");
    mHandle = dlopen(kRbsLib, RTLD_NOW);
    if (mHandle == nullptr) {
        ALOGE("No valid fingerprint module");
        return false;
    }

    mDevice = new rbs_fingerprint_device_t{};

#define RBS_SYM(name) \
    mDevice->name = reinterpret_cast<typeof(mDevice->name)>(dlsym(mHandle, #name))

    RBS_SYM(rbs_initialize);
    RBS_SYM(rbs_uninitialize);
    RBS_SYM(rbs_cancel);
    RBS_SYM(rbs_active_user_group);
    RBS_SYM(rbs_chk_secure_id);
    RBS_SYM(rbs_pre_enroll);
    RBS_SYM(rbs_enroll);
    RBS_SYM(rbs_chk_auth_token);
    RBS_SYM(rbs_authenticator);
    RBS_SYM(rbs_remove_fingerprint);
    RBS_SYM(rbs_get_fingerprint_ids);
    RBS_SYM(rbs_get_authenticator_id);
    RBS_SYM(rbs_set_on_callback_proc);

#undef RBS_SYM

    if (mDevice->rbs_set_on_callback_proc == nullptr || mDevice->rbs_initialize == nullptr) {
        ALOGE("libRbsFlow.so is missing required symbols");
        delete mDevice;
        mDevice = nullptr;
        return false;
    }

    mDevice->rbs_set_on_callback_proc(reinterpret_cast<void*>(RbsDevice::onCallback));

    int err = mDevice->rbs_initialize(0, 0);
    if (err != 0) {
        ALOGE("Can't open fingerprint, error %d", err);
        delete mDevice;
        mDevice = nullptr;
        return false;
    }

    return true;
}

void RbsDevice::close() {
    if (mDevice == nullptr) return;
    int err = mDevice->rbs_uninitialize();
    if (err != 0) ALOGE("Can't close fingerprint module, error: %d", err);
    delete mDevice;
    mDevice = nullptr;
}

uint64_t RbsDevice::preEnroll() {
    // Only the QSEE variant can ask the TEE for a challenge; seen on exynos 9610.
    mChallenge = static_cast<uint64_t>(rand()) | (static_cast<uint64_t>(rand()) << 0x20);
    return mChallenge;
}

int RbsDevice::enroll(const hw_auth_token_t* authToken, uint32_t gid, uint32_t /*timeoutSec*/) {
    if (authToken == nullptr || authToken->timestamp == 0) {
        ALOGE("HAT is null");
        return -ENOENT;
    }

    if (authToken->challenge != mChallenge) {
        ALOGE("Challenge does not match");
        return -EINVAL;
    }

    if (authToken->version != 0) {
        ALOGE("Invalid HAT version = %d", authToken->version);
        return -EINVAL;
    }

    if ((authToken->challenge != mChallenge) &&
        !(authToken->authenticator_type & HW_AUTH_FINGERPRINT)) {
        ALOGE("Invalid authenticator type");
        return -EINVAL;
    }

    int rc = mDevice->rbs_chk_auth_token(authToken, sizeof(hw_auth_token_t));
    if (rc != 0) {
        ALOGE("Auth token check failed, error %d", rc);
        return rc;
    }

    rc = mDevice->rbs_chk_secure_id(gid, authToken->user_id);
    if (rc != 0) {
        ALOGD("Secure ID check failed, error %d", rc);
        if (rc != 0x21) return -EINVAL;

        rc = mDevice->rbs_remove_fingerprint(gid, 0);
        if (rc != 0) {
            ALOGE("Remove all fingerprints failed, error %d", rc);
            return -EINVAL;
        }
        // After nuking everything, check if Secure ID is okay again
        rc = mDevice->rbs_chk_secure_id(gid, authToken->user_id);
        if (rc != 0) {
            ALOGD("Secure ID check failed, error %d", rc);
            return -EINVAL;
        }
        ALOGD("Removed all fingerprints and secure ID check OK");
    }

    int preEnrollRc = 0;
    do {
        preEnrollRc = mDevice->rbs_pre_enroll(gid, rand());
        if (preEnrollRc == 0) {
            rc = mDevice->rbs_enroll();
            if (rc == 0) return 0;
            notifyError(FINGERPRINT_ERROR_CANCELED);
            if (rc != 4) return 0;
            notifyError(FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
            return 0;
        }
    } while (preEnrollRc != 11);

    ALOGE("Pre-enroll failed, no space");
    notifyError(FINGERPRINT_ERROR_CANCELED);
    return 0;
}

int RbsDevice::postEnroll() {
    mChallenge = 0;
    return 0;
}

uint64_t RbsDevice::getAuthenticatorId() {
    uint64_t authenticatorId = 0;
    mDevice->rbs_get_authenticator_id(&authenticatorId);
    return authenticatorId;
}

int RbsDevice::cancel() {
    return mDevice->rbs_cancel();
}

int RbsDevice::enumerate() {
    uint32_t numFids = 0;
    uint32_t fids[kMaxFingers] = {};

    if (mGid == kNoGid) {
        ALOGE("User ID empty");
        return -EINVAL;
    }

    int rc = mDevice->rbs_get_fingerprint_ids(mGid, fids, &numFids);
    if (rc != 0) {
        ALOGE("Enumerate failed, error %d", rc);
        return rc;
    }

    if (numFids == 0) {
        notifyEnumerate(0, mGid, 0);
    } else {
        for (uint32_t i = 0; i < numFids; i++) {
            notifyEnumerate(fids[i], mGid, numFids - i - 1);
        }
    }

    return 0;
}

int RbsDevice::remove(uint32_t gid, uint32_t fid) {
    uint32_t numFids = 0;
    uint32_t fids[kMaxFingers] = {};

    int rc = mDevice->rbs_get_fingerprint_ids(gid, fids, &numFids);
    if (rc != 0) {
        notifyError(FINGERPRINT_ERROR_UNABLE_TO_REMOVE);
        return 0;
    }

    if (numFids == 0) {
        ALOGD("No fingerprints registered");
        notifyRemoved(0, gid, 0);
        return 0;
    }

    /*
     * The RBS API removes every fingerprint when the FID is 0, so there is no
     * need to loop here, only over the callbacks.
     */
    rc = mDevice->rbs_remove_fingerprint(gid, fid);
    if (rc == 0) {
        if (fid != 0) {
            rc = mDevice->rbs_get_fingerprint_ids(gid, fids, &numFids);
            if (rc == 0) notifyError(FINGERPRINT_ERROR_UNABLE_TO_REMOVE);
        } else {
            for (uint32_t i = 0; i < numFids; i++) {
                notifyRemoved(fids[i], gid, numFids - i - 1);
            }
            notifyError(FINGERPRINT_ERROR_CANCELED);
            return 0;
        }
    }

    /*
     * Removal can fail while stock still reports the fingerprint as removed,
     * so that the framework invalidates it rather than leaving it dangling.
     */
    notifyRemoved(fid, gid, 0);
    return 0;
}

int RbsDevice::setActiveGroup(uint32_t gid, const char* storePath) {
    mGid = gid;

    if (storePath == nullptr) return -EINVAL;
    std::string path(storePath);
    if (path.empty() || path.size() >= PATH_MAX) {
        ALOGE("Bad path length: %zd", path.size());
        return -EINVAL;
    }

    if (StartsWith(path, "/data/system/users/")) {
        path = "/data/vendor_de/" + path.substr(strlen("/data/system/users/"));
    }

    if (access(path.c_str(), W_OK)) return -EINVAL;

    return mDevice->rbs_active_user_group(gid, path.c_str());
}

int RbsDevice::authenticate(uint64_t operationId, uint32_t gid) {
    mGid = gid;

    int rc = mDevice->rbs_authenticator(gid, 0, 0, operationId);
    if (rc != 0) {
        notifyError(FINGERPRINT_ERROR_CANCELED);
        if (rc == 4) notifyError(FINGERPRINT_ERROR_HW_UNAVAILABLE);
    }

    return rc;
}

void RbsDevice::onCallback(uint32_t eventId, uint32_t value1, uint32_t value2, void* buffer,
                           uint32_t /*bufferSize*/) {
    RbsDevice* self = sInstance;
    if (self == nullptr) {
        ALOGE("Receiving callbacks before the device is open.");
        return;
    }
    std::lock_guard<std::mutex> lock(self->mCallbackMutex);

    switch (eventId) {
        // Error
        case 0x3eb:
        case 0x401:
            self->notifyError(FINGERPRINT_ERROR_CANCELED);
            break;
        case 0x40e:
            self->notifyError(FINGERPRINT_ERROR_TIMEOUT);
            break;
        // Acquired
        case 0x3ec:
        case 0x3ed:
            self->notifyAcquired(FINGERPRINT_ACQUIRED_TOO_SLOW);
            break;
        case 0x3ee:
        case 0x3ef:
            self->notifyAcquired(FINGERPRINT_ACQUIRED_VENDOR_BASE);
            break;
        case 0x3f5:
            self->notifyAcquired(FINGERPRINT_ACQUIRED_INSUFFICIENT);
            break;
        case 0x3f7:
        case 0x3f8:
            self->notifyAcquired(FINGERPRINT_ACQUIRED_PARTIAL);
            break;
        case 0x3f9:
        case 0x3fa:
        case 0x3fb:
            self->notifyAcquired(FINGERPRINT_ACQUIRED_TOO_FAST);
            break;
        case 0x3fe:
            self->notifyAcquired(FINGERPRINT_ACQUIRED_GOOD);
            break;
        // Enrolling
        case 0x40d:
            self->notifyEnrollResult(value1, self->mGid, value2);
            break;
        // Authenticated
        case 0x3f2:
        case 0x3f3: {
            uint32_t gid = value1;
            uint32_t fid = value2;
            self->notifyAuthenticated(fid, gid, reinterpret_cast<const uint8_t*>(buffer),
                                      fid != 0 ? sizeof(hw_auth_token_t) : 0);
        } break;
    }
}

}  // namespace aidl::android::hardware::biometrics::fingerprint
