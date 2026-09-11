/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vendor/oppo/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>

#include <vector>

#include "FingerprintDevice.h"

namespace aidl::android::hardware::biometrics::fingerprint {

namespace oppo_fp = ::vendor::oppo::hardware::biometrics::fingerprint::V2_1;

/*
 * FPC sensor, reached through OPPO's own HIDL service. Ported from the
 * @2.1-service.r5x shim in fingerprint/.
 */
class OppoDevice : public FingerprintDevice {
  public:

    bool open() override;

    uint64_t preEnroll() override;
    int enroll(const hw_auth_token_t* hat, uint32_t gid, uint32_t timeoutSec) override;
    int postEnroll() override;
    uint64_t getAuthenticatorId() override;
    int cancel() override;
    int enumerate() override;
    int remove(uint32_t gid, uint32_t fid) override;
    int setActiveGroup(uint32_t gid, const char* storePath) override;
    int authenticate(uint64_t operationId, uint32_t gid) override;

    // Called from the HIDL callback object.
    void onError(int32_t error);
    void onAcquired(int32_t info);
    void onEnrollResult(uint32_t fid, uint32_t gid, uint32_t remaining);
    void onRemoved(uint32_t fid, uint32_t gid, uint32_t remaining);
    void onEnumerate(uint32_t fid, uint32_t gid, uint32_t remaining);
    void onAuthenticated(uint32_t fid, uint32_t gid, const uint8_t* token, size_t tokenSize);
    void onSyncTemplates(const std::vector<uint32_t>& fids);

  private:
    ::android::sp<oppo_fp::IBiometricsFingerprint> mService;

    // The OPPO service does not always answer cancel() or enumerate(), so the
    // shim tracks whether it did and synthesises the missing callback.
    bool mReceivedCancel = false;
    bool mReceivedEnumerate = false;
    std::vector<uint32_t> mKnownFingers;
    uint32_t mGid = 0;
};

}  // namespace aidl::android::hardware::biometrics::fingerprint
