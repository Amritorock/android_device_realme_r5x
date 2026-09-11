/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <hardware/fingerprint.h>
#include <hardware/hw_auth_token.h>

#include <cstring>
#include <functional>

namespace aidl::android::hardware::biometrics::fingerprint {

/*
 * The device layer behind the AIDL service. r5x ships two different sensors,
 * so the operations Session needs are abstracted here and every backend
 * reports events as a fingerprint_msg_t, which is what Session already
 * consumes.
 */
class FingerprintDevice {
  public:
    using NotifyFn = std::function<void(const fingerprint_msg_t*)>;

    virtual ~FingerprintDevice() = default;

    virtual bool open() = 0;
    virtual void close() {}

    void setNotify(NotifyFn notify) { mNotify = std::move(notify); }

    virtual uint64_t preEnroll() = 0;
    virtual int enroll(const hw_auth_token_t* hat, uint32_t gid, uint32_t timeoutSec) = 0;
    virtual int postEnroll() = 0;
    virtual uint64_t getAuthenticatorId() = 0;
    virtual int cancel() = 0;
    virtual int enumerate() = 0;
    virtual int remove(uint32_t gid, uint32_t fid) = 0;
    virtual int setActiveGroup(uint32_t gid, const char* storePath) = 0;
    virtual int authenticate(uint64_t operationId, uint32_t gid) = 0;

  protected:
    void notifyError(int32_t error) {
        fingerprint_msg_t msg{};
        msg.type = FINGERPRINT_ERROR;
        msg.data.error = static_cast<fingerprint_error_t>(error);
        dispatch(&msg);
    }

    void notifyAcquired(int32_t info) {
        fingerprint_msg_t msg{};
        msg.type = FINGERPRINT_ACQUIRED;
        msg.data.acquired.acquired_info = static_cast<fingerprint_acquired_info_t>(info);
        dispatch(&msg);
    }

    void notifyEnrollResult(uint32_t fid, uint32_t gid, uint32_t remaining) {
        fingerprint_msg_t msg{};
        msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
        msg.data.enroll.finger.fid = fid;
        msg.data.enroll.finger.gid = gid;
        msg.data.enroll.samples_remaining = remaining;
        dispatch(&msg);
    }

    void notifyRemoved(uint32_t fid, uint32_t gid, uint32_t remaining) {
        fingerprint_msg_t msg{};
        msg.type = FINGERPRINT_TEMPLATE_REMOVED;
        msg.data.removed.finger.fid = fid;
        msg.data.removed.finger.gid = gid;
        msg.data.removed.remaining_templates = remaining;
        dispatch(&msg);
    }

    void notifyEnumerate(uint32_t fid, uint32_t gid, uint32_t remaining) {
        fingerprint_msg_t msg{};
        msg.type = FINGERPRINT_TEMPLATE_ENUMERATING;
        msg.data.enumerated.finger.fid = fid;
        msg.data.enumerated.finger.gid = gid;
        msg.data.enumerated.remaining_templates = remaining;
        dispatch(&msg);
    }

    void notifyAuthenticated(uint32_t fid, uint32_t gid, const uint8_t* token, size_t tokenSize) {
        fingerprint_msg_t msg{};
        msg.type = FINGERPRINT_AUTHENTICATED;
        msg.data.authenticated.finger.fid = fid;
        msg.data.authenticated.finger.gid = gid;
        if (token != nullptr && tokenSize >= sizeof(hw_auth_token_t)) {
            memcpy(&msg.data.authenticated.hat, token, sizeof(hw_auth_token_t));
        }
        dispatch(&msg);
    }

  private:
    void dispatch(const fingerprint_msg_t* msg) {
        if (mNotify) mNotify(msg);
    }

    NotifyFn mNotify;
};

}  // namespace aidl::android::hardware::biometrics::fingerprint
