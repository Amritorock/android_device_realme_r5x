/*
 * Copyright (C) 2024 The LineageOS Project
 *               2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Fingerprint.h"

#include <android-base/logging.h>
#include <android-base/properties.h>

#include "OppoDevice.h"
#include "RbsDevice.h"

namespace aidl::android::hardware::biometrics::fingerprint {

namespace {
constexpr int MAX_ENROLLMENTS_PER_USER = 5;
constexpr int SENSOR_ID = 0;
constexpr char HW_COMPONENT_ID[] = "fingerprintSensor";
constexpr char HW_VERSION[] = "vendor/model/revision";
constexpr char FW_VERSION[] = "1.01";
constexpr char SERIAL_NUMBER[] = "00000001";
constexpr char SW_COMPONENT_ID[] = "matchingAlgorithm";
constexpr char SW_VERSION[] = "vendor/version/revision";

// Set by the vendor to E_520 when the Egistec sensor is fitted.
constexpr char FP_ID_PROP[] = "persist.vendor.fingerprint.fp_id";
constexpr char FP_ID_EGISTEC[] = "E_520";
}  // namespace

static Fingerprint* sInstance;

Fingerprint::Fingerprint() : mDevice(openHal()) {
    sInstance = this;  // keep track of the most recent instance
}

Fingerprint::~Fingerprint() {
    ALOGV("~Fingerprint()");
    if (mDevice == nullptr) {
        ALOGE("No valid device");
        return;
    }
    mDevice->close();
    mDevice = nullptr;
}

/*
 * r5x ships with either the Egistec RBS sensor or an FPC one behind OPPO's own
 * HIDL service, so pick the backend the way the vendor init scripts do and fall
 * back to whichever one is actually there.
 */
std::unique_ptr<FingerprintDevice> Fingerprint::openHal() {
    const std::string fpId = ::android::base::GetProperty(FP_ID_PROP, "");
    const bool preferRbs = (fpId == FP_ID_EGISTEC);
    ALOGI("%s is '%s', trying %s first", FP_ID_PROP, fpId.c_str(), preferRbs ? "RBS" : "OPPO");

    std::unique_ptr<FingerprintDevice> device;
    if (preferRbs) {
        device = std::make_unique<RbsDevice>();
        if (!device->open()) {
            ALOGE("RBS sensor not usable, falling back to the OPPO service");
            device = std::make_unique<OppoDevice>();
            if (!device->open()) device = nullptr;
        }
    } else {
        device = std::make_unique<OppoDevice>();
        if (!device->open()) {
            ALOGE("OPPO service not usable, falling back to the RBS sensor");
            device = std::make_unique<RbsDevice>();
            if (!device->open()) device = nullptr;
        }
    }

    if (device == nullptr) {
        ALOGE("No usable fingerprint backend");
        return nullptr;
    }

    device->setNotify(Fingerprint::notify);
    return device;
}

void Fingerprint::notify(const fingerprint_msg_t* msg) {
    Fingerprint* thisPtr = sInstance;
    if (thisPtr == nullptr || thisPtr->mSession == nullptr || thisPtr->mSession->isClosed()) {
        ALOGE("Receiving callbacks before a session is opened.");
        return;
    }
    thisPtr->mSession->notify(msg);
}

ndk::ScopedAStatus Fingerprint::getSensorProps(std::vector<SensorProps>* out) {
    std::vector<common::ComponentInfo> componentInfo = {
            {HW_COMPONENT_ID, HW_VERSION, FW_VERSION, SERIAL_NUMBER, "" /* softwareVersion */},
            {SW_COMPONENT_ID, "" /* hardwareVersion */, "" /* firmwareVersion */,
             "" /* serialNumber */, SW_VERSION}};

    common::CommonProps commonProps = {SENSOR_ID, common::SensorStrength::STRONG,
                                       MAX_ENROLLMENTS_PER_USER, componentInfo};

    /*
     * A capacitive sensor on the back panel: no on-display location, no
     * navigation gestures and no detect-interaction support.
     */
    *out = {{commonProps, FingerprintSensorType::REAR, {} /* sensorLocations */,
             false /* supportsNavigationGestures */, false /* supportsDetectInteraction */,
             false /* halHandlesDisplayTouches */, false /* halControlsIllumination */,
             std::nullopt /* touchDetectionParameters */}};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Fingerprint::createSession(int32_t /*sensorId*/, int32_t userId,
                                              const std::shared_ptr<ISessionCallback>& cb,
                                              std::shared_ptr<ISession>* out) {
    CHECK(mSession == nullptr || mSession->isClosed()) << "Open session already exists!";

    mSession = SharedRefBase::make<Session>(mDevice.get(), userId, cb, mLockoutTracker);
    *out = mSession;

    mSession->linkToDeath(cb->asBinder().get());

    return ndk::ScopedAStatus::ok();
}

}  // namespace aidl::android::hardware::biometrics::fingerprint
