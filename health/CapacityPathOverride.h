/*
 * Copyright (C) 2021 The Android Open Source Project
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

#pragma once

#include <android-base/logging.h>
#include <utils/String8.h>

#include <unistd.h>

namespace aidl::android::hardware::health::r5x {

inline void OverrideHealthdConfigPaths(healthd_config* config) {
    static constexpr const char* kBmsCapacityPath = "/sys/class/power_supply/bms/capacity";

    if (access(kBmsCapacityPath, R_OK) == 0) {
        config->batteryCapacityPath = ::android::String8(kBmsCapacityPath);
        LOG(INFO) << "health-r5x: batteryCapacityPath -> " << kBmsCapacityPath;
    } else {
        LOG(WARNING) << "health-r5x: " << kBmsCapacityPath
                      << " not accessible, keeping auto-detected capacity path";
    }
}

}  // namespace aidl::android::hardware::health::r5x
