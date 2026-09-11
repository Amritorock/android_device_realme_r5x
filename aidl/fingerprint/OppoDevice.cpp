/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "android.hardware.biometrics.fingerprint-service.r5x"

#include "OppoDevice.h"

#include <cerrno>
#include <cstring>
#include <hidl/HidlTransportSupport.h>
#include <log/log.h>
#include <unistd.h>

namespace aidl::android::hardware::biometrics::fingerprint {

using ::android::sp;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

namespace {

int32_t toLegacyAcquired(oppo_fp::FingerprintAcquiredInfo info) {
    switch (info) {
        case oppo_fp::FingerprintAcquiredInfo::ACQUIRED_PARTIAL:
            return FINGERPRINT_ACQUIRED_PARTIAL;
        case oppo_fp::FingerprintAcquiredInfo::ACQUIRED_INSUFFICIENT:
            return FINGERPRINT_ACQUIRED_INSUFFICIENT;
        case oppo_fp::FingerprintAcquiredInfo::ACQUIRED_IMAGER_DIRTY:
            return FINGERPRINT_ACQUIRED_IMAGER_DIRTY;
        case oppo_fp::FingerprintAcquiredInfo::ACQUIRED_TOO_SLOW:
            return FINGERPRINT_ACQUIRED_TOO_SLOW;
        case oppo_fp::FingerprintAcquiredInfo::ACQUIRED_TOO_FAST:
            return FINGERPRINT_ACQUIRED_TOO_FAST;
        case oppo_fp::FingerprintAcquiredInfo::ACQUIRED_VENDOR:
            return FINGERPRINT_ACQUIRED_VENDOR_BASE;
        case oppo_fp::FingerprintAcquiredInfo::ACQUIRED_GOOD:
        default:
            return FINGERPRINT_ACQUIRED_GOOD;
    }
}

int32_t toLegacyError(oppo_fp::FingerprintError error) {
    switch (error) {
        case oppo_fp::FingerprintError::ERROR_HW_UNAVAILABLE:
            return FINGERPRINT_ERROR_HW_UNAVAILABLE;
        case oppo_fp::FingerprintError::ERROR_UNABLE_TO_PROCESS:
            return FINGERPRINT_ERROR_UNABLE_TO_PROCESS;
        case oppo_fp::FingerprintError::ERROR_TIMEOUT:
            return FINGERPRINT_ERROR_TIMEOUT;
        case oppo_fp::FingerprintError::ERROR_NO_SPACE:
            return FINGERPRINT_ERROR_NO_SPACE;
        case oppo_fp::FingerprintError::ERROR_CANCELED:
            return FINGERPRINT_ERROR_CANCELED;
        case oppo_fp::FingerprintError::ERROR_UNABLE_TO_REMOVE:
            return FINGERPRINT_ERROR_UNABLE_TO_REMOVE;
        case oppo_fp::FingerprintError::ERROR_LOCKOUT:
            return FINGERPRINT_ERROR_LOCKOUT;
        case oppo_fp::FingerprintError::ERROR_VENDOR:
            return FINGERPRINT_ERROR_VENDOR_BASE;
        case oppo_fp::FingerprintError::ERROR_NO_ERROR:
        default:
            return 0;
    }
}

int toLegacyStatus(oppo_fp::RequestStatus status) {
    return status == oppo_fp::RequestStatus::SYS_OK ? 0 : -EINVAL;
}

class OppoClientCallback : public oppo_fp::IBiometricsFingerprintClientCallback {
  public:
    explicit OppoClientCallback(OppoDevice* device) : mDevice(device) {}

    Return<void> onEnrollResult(uint64_t, uint32_t fingerId, uint32_t groupId,
                                uint32_t remaining) override {
        mDevice->onEnrollResult(fingerId, groupId, remaining);
        return Void();
    }

    Return<void> onAcquired(uint64_t, oppo_fp::FingerprintAcquiredInfo acquiredInfo,
                            int32_t) override {
        mDevice->onAcquired(toLegacyAcquired(acquiredInfo));
        return Void();
    }

    Return<void> onAuthenticated(uint64_t, uint32_t fingerId, uint32_t groupId,
                                 const hidl_vec<uint8_t>& token) override {
        mDevice->onAuthenticated(fingerId, groupId, token.data(), token.size());
        return Void();
    }

    Return<void> onError(uint64_t, oppo_fp::FingerprintError error, int32_t) override {
        mDevice->onError(toLegacyError(error));
        return Void();
    }

    Return<void> onRemoved(uint64_t, uint32_t fingerId, uint32_t groupId,
                           uint32_t remaining) override {
        mDevice->onRemoved(fingerId, groupId, remaining);
        return Void();
    }

    Return<void> onEnumerate(uint64_t, uint32_t fingerId, uint32_t groupId,
                             uint32_t remaining) override {
        mDevice->onEnumerate(fingerId, groupId, remaining);
        return Void();
    }

    Return<void> onSyncTemplates(uint64_t, const hidl_vec<uint32_t>& fingerId, uint32_t) override {
        mDevice->onSyncTemplates(std::vector<uint32_t>(fingerId.begin(), fingerId.end()));
        return Void();
    }

    // OPPO extensions with no AIDL equivalent on a rear sensor.
    Return<void> onTouchUp(uint64_t) override { return Void(); }
    Return<void> onTouchDown(uint64_t) override { return Void(); }
    Return<void> onFingerprintCmd(int32_t, const hidl_vec<uint32_t>&, uint32_t) override {
        return Void();
    }
    Return<void> onImageInfoAcquired(uint32_t, uint32_t, uint32_t) override { return Void(); }
    Return<void> onMonitorEventTriggered(uint32_t, const hidl_string&) override { return Void(); }
    Return<void> onEngineeringInfoUpdated(uint32_t, const hidl_vec<uint32_t>&,
                                          const hidl_vec<hidl_string>&) override {
        return Void();
    }

  private:
    OppoDevice* mDevice;
};

}  // namespace

bool OppoDevice::open() {
    /*
     * The service is AIDL, so only the NDK threadpool is running. Callbacks
     * from the OPPO service arrive over hwbinder and need a HIDL threadpool of
     * their own, otherwise nothing is ever delivered and every operation the
     * framework schedules hangs waiting for a result.
     */
    ::android::hardware::configureRpcThreadpool(1, false /* callerWillJoin */);

    /*
     * fps_hal comes up in late_start alongside us. Keep the wait short: this
     * runs before IFingerprint is registered, so blocking here holds up
     * everything waiting on the service.
     */
    for (int i = 0; i < 30; i++) {
        mService = oppo_fp::IBiometricsFingerprint::tryGetService();
        if (mService != nullptr) break;
        usleep(500000);
    }

    if (mService == nullptr) {
        ALOGE("Can't get OPPO fingerprint service");
        return false;
    }

    mService->setNotify(new OppoClientCallback(this));
    return true;
}

uint64_t OppoDevice::preEnroll() {
    return mService->preEnroll();
}

int OppoDevice::enroll(const hw_auth_token_t* hat, uint32_t gid, uint32_t timeoutSec) {
    ::android::hardware::hidl_array<uint8_t, 69> token;
    memcpy(token.data(), hat, sizeof(hw_auth_token_t));
    return toLegacyStatus(mService->enroll(token, gid, timeoutSec));
}

int OppoDevice::postEnroll() {
    return toLegacyStatus(mService->postEnroll());
}

uint64_t OppoDevice::getAuthenticatorId() {
    return mService->getAuthenticatorId();
}

int OppoDevice::cancel() {
    mReceivedCancel = false;
    int ret = toLegacyStatus(mService->cancel());
    if (!mReceivedCancel) {
        ALOGD("No cancel from the OPPO service, sending our own");
        notifyError(FINGERPRINT_ERROR_CANCELED);
    }
    return ret;
}

int OppoDevice::enumerate() {
    mReceivedEnumerate = false;
    int ret = toLegacyStatus(mService->enumerate());
    if (ret != 0 || mReceivedEnumerate) return ret;

    // The OPPO service stays quiet, so replay what onSyncTemplates last told us.
    size_t remaining = mKnownFingers.size();
    if (remaining == 0) {
        notifyEnumerate(0, mGid, 0);
    } else {
        for (auto fid : mKnownFingers) {
            notifyEnumerate(fid, mGid, --remaining);
        }
    }
    return ret;
}

int OppoDevice::remove(uint32_t gid, uint32_t fid) {
    return toLegacyStatus(mService->remove(gid, fid));
}

int OppoDevice::setActiveGroup(uint32_t gid, const char* storePath) {
    mGid = gid;
    return toLegacyStatus(mService->setActiveGroup(gid, hidl_string(storePath)));
}

int OppoDevice::authenticate(uint64_t operationId, uint32_t gid) {
    mGid = gid;
    return toLegacyStatus(mService->authenticate(operationId, gid));
}

void OppoDevice::onError(int32_t error) {
    // The legacy enum has no "no error" member, so toLegacyError maps it to 0.
    if (error == 0) return;
    if (error == FINGERPRINT_ERROR_CANCELED) mReceivedCancel = true;
    notifyError(error);
}

void OppoDevice::onAcquired(int32_t info) {
    notifyAcquired(info);
}

void OppoDevice::onEnrollResult(uint32_t fid, uint32_t gid, uint32_t remaining) {
    notifyEnrollResult(fid, gid, remaining);
}

void OppoDevice::onRemoved(uint32_t fid, uint32_t gid, uint32_t remaining) {
    notifyRemoved(fid, gid, remaining);
}

void OppoDevice::onEnumerate(uint32_t fid, uint32_t gid, uint32_t remaining) {
    mReceivedEnumerate = true;
    notifyEnumerate(fid, gid, remaining);
}

void OppoDevice::onAuthenticated(uint32_t fid, uint32_t gid, const uint8_t* token,
                                 size_t tokenSize) {
    notifyAuthenticated(fid, gid, token, tokenSize);
}

void OppoDevice::onSyncTemplates(const std::vector<uint32_t>& fids) {
    mKnownFingers = fids;
}

}  // namespace aidl::android::hardware::biometrics::fingerprint
