/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HARDWARE_BIOMETRICS_FINGERPRINT_V2_3_BIOMETRICSFINGERPRINT_H
#define ANDROID_HARDWARE_BIOMETRICS_FINGERPRINT_V2_3_BIOMETRICSFINGERPRINT_H

#include <android/hardware/biometrics/fingerprint/2.3/IBiometricsFingerprint.h>
#include <android/log.h>
#include <hardware/fingerprint.h>
#include <hardware/hardware.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <log/log.h>
#include "UdfpsHandler.h"

#ifdef _HAS_QSEE

extern "C" struct QSEECom_handle { unsigned char* ion_sbuffer; };

extern "C" struct ets_masterkey_response {
    uint32_t rc;
    uint32_t pad;
    uint32_t size;
    uint32_t masterkey[256];
};

#endif

namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_3 {
namespace implementation {

using ::android::hardware::biometrics::fingerprint::V2_3::IBiometricsFingerprint;
using IBiometricsFingerprintClientCallback =
        ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallback;
using RequestStatus = android::hardware::biometrics::fingerprint::V2_1::RequestStatus;
using FingerprintAcquiredInfo =
        ::android::hardware::biometrics::fingerprint::V2_1::FingerprintAcquiredInfo;
using FingerprintError = ::android::hardware::biometrics::fingerprint::V2_1::FingerprintError;

using ::android::sp;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

struct BiometricsFingerprint : public IBiometricsFingerprint {
  public:
    BiometricsFingerprint();
    ~BiometricsFingerprint();

    // Method to wrap legacy HAL with BiometricsFingerprint class
    static IBiometricsFingerprint* getInstance();

    // Methods from ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint
    // follow.
    Return<uint64_t> setNotify(
            const sp<IBiometricsFingerprintClientCallback>& clientCallback) override;
    Return<uint64_t> preEnroll() override;
    Return<RequestStatus> enroll(const hidl_array<uint8_t, 69>& hat, uint32_t gid,
                                 uint32_t timeoutSec) override;
    Return<RequestStatus> postEnroll() override;
    Return<uint64_t> getAuthenticatorId() override;
    Return<RequestStatus> cancel() override;
    Return<RequestStatus> enumerate() override;
    Return<RequestStatus> remove(uint32_t gid, uint32_t fid) override;
    Return<RequestStatus> setActiveGroup(uint32_t gid, const hidl_string& storePath) override;
    Return<RequestStatus> authenticate(uint64_t operationId, uint32_t gid) override;

    // ::V2_3::IBiometricsFingerprint follow.
    Return<bool> isUdfps(uint32_t sensorId) override;
    Return<void> onFingerDown(uint32_t x, uint32_t y, float minor, float major) override;
    Return<void> onFingerUp() override;

  private:
    static rbs_fingerprint_device_t* openHal();
    static void notify(uint32_t eventId, uint32_t value1, uint32_t value2, void* buffer,
                       uint32_t buffer_size); /* Static callback for legacy HAL implementation */
    static Return<RequestStatus> ErrorFilter(int32_t error);
    void onErrorCallback(FingerprintError error, uint32_t vendorCode);
    void onEnumerateCallback(uint32_t fid, uint32_t gid, uint32_t samples_remaining);
    void onRemovedCallback(uint32_t fid, uint32_t gid, uint32_t samples_remaining);
    void doExtraApi(uint32_t param);
    void notifyScanStart(void);
    void notifyScanStop(void);
    static BiometricsFingerprint* sInstance;
    static int getSecureKey(void* masterkey, uint32_t masterkey_size);

    std::mutex mClientCallbackMutex;
    sp<IBiometricsFingerprintClientCallback> mClientCallback;
    rbs_fingerprint_device_t* mDevice;
    uint32_t mGid = 0;
    uint64_t mChallenge = 0;
    uint64_t mOperationId = 0;

    bool mIsUdfps;
    UdfpsHandlerFactory* mUdfpsHandlerFactory;
    UdfpsHandler* mUdfpsHandler;
};

}  // namespace implementation
}  // namespace V2_3
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_BIOMETRICS_FINGERPRINT_V2_3_BIOMETRICSFINGERPRINT_H
