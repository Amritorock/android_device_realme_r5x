/*
 * Copyright (C) 2024 The LineageOS Project
 *               2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/biometrics/fingerprint/BnFingerprint.h>

#include "FingerprintConfig.h"
#include "LockoutTracker.h"
#include "Session.h"

using ::aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType;
using ::aidl::android::hardware::biometrics::fingerprint::ISession;
using ::aidl::android::hardware::biometrics::fingerprint::ISessionCallback;
using ::aidl::android::hardware::biometrics::fingerprint::SensorProps;

namespace aidl::android::hardware::biometrics::fingerprint {

class Fingerprint : public BnFingerprint {
  public:
    Fingerprint(std::shared_ptr<FingerprintConfig> config);
    ~Fingerprint();

    ndk::ScopedAStatus getSensorProps(std::vector<SensorProps>* _aidl_return) override;
    ndk::ScopedAStatus createSession(int32_t sensorId, int32_t userId,
                                     const std::shared_ptr<ISessionCallback>& cb,
                                     std::shared_ptr<ISession>* out) override;

  private:
    fingerprint_device_t* openHal(void);
    std::vector<SensorLocation> getSensorLocations();
    static void notify(const fingerprint_msg_t* msg);

    std::shared_ptr<FingerprintConfig> mConfig;
    std::shared_ptr<Session> mSession;
    LockoutTracker mLockoutTracker;
    FingerprintSensorType mSensorType;

    fingerprint_device_t* mDevice;
};

}  // namespace aidl::android::hardware::biometrics::fingerprint
