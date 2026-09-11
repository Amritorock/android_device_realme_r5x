/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <rbs_fingerprint.h>

#include <mutex>

#include "FingerprintDevice.h"

namespace aidl::android::hardware::biometrics::fingerprint {

/*
 * Egistec RBS sensor, driven through libRbsFlow.so. Ported from the
 * @2.3-service.rbs HIDL HAL in hidl/rbs.
 */
class RbsDevice : public FingerprintDevice {
  public:

    bool open() override;
    void close() override;

    uint64_t preEnroll() override;
    int enroll(const hw_auth_token_t* hat, uint32_t gid, uint32_t timeoutSec) override;
    int postEnroll() override;
    uint64_t getAuthenticatorId() override;
    int cancel() override;
    int enumerate() override;
    int remove(uint32_t gid, uint32_t fid) override;
    int setActiveGroup(uint32_t gid, const char* storePath) override;
    int authenticate(uint64_t operationId, uint32_t gid) override;

  private:
    static void onCallback(uint32_t eventId, uint32_t value1, uint32_t value2, void* buffer,
                           uint32_t bufferSize);

    static RbsDevice* sInstance;

    void* mHandle = nullptr;
    rbs_fingerprint_device_t* mDevice = nullptr;
    uint64_t mChallenge = 0;
    uint32_t mGid = 9999;
    std::mutex mCallbackMutex;
};

}  // namespace aidl::android::hardware::biometrics::fingerprint
