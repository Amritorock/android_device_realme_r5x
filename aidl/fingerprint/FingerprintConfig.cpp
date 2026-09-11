/*
 * Copyright (C) 2024 The Android Open Source Project
 *               2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "FingerprintConfig"

#include "FingerprintConfig.h"

#include <android-base/logging.h>

#include <fingerprint.sysprop.h>

using namespace ::android::fingerprint::lineage;

namespace aidl::android::hardware::biometrics::fingerprint {

// Wrapper to system property access functions
#define CREATE_GETTER_SETTER_WRAPPER(_NAME_, _T_)                  \
    ConfigValue _NAME_##Getter() {                                 \
        return FingerprintHalProperties::_NAME_();                 \
    }                                                              \
    bool _NAME_##Setter(const ConfigValue& v) {                    \
        return FingerprintHalProperties::_NAME_(std::get<_T_>(v)); \
    }

CREATE_GETTER_SETTER_WRAPPER(type, OptString)
CREATE_GETTER_SETTER_WRAPPER(sensor_id, OptInt32)
CREATE_GETTER_SETTER_WRAPPER(sensor_location, OptString)
CREATE_GETTER_SETTER_WRAPPER(sensor_strength, OptInt32)
CREATE_GETTER_SETTER_WRAPPER(navigation_gesture, OptBool)
CREATE_GETTER_SETTER_WRAPPER(detect_interaction, OptBool)

// Name, Getter, Setter, Parser and default value
#define NGS(_NAME_) #_NAME_, _NAME_##Getter, _NAME_##Setter
static Config::Data configData[] = {
        {NGS(type), &Config::parseString, ""},
        {NGS(sensor_id), &Config::parseInt32, "0"},
        {NGS(sensor_location), &Config::parseString, ""},
        {NGS(sensor_strength), &Config::parseInt32, "2"},  // STRONG
        {NGS(navigation_gesture), &Config::parseBool, "false"},
        {NGS(detect_interaction), &Config::parseBool, "false"},
};

Config::Data* FingerprintConfig::getConfigData(int* size) {
    *size = sizeof(configData) / sizeof(configData[0]);
    return configData;
}

}  // namespace aidl::android::hardware::biometrics::fingerprint
