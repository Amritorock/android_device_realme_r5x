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

#include <android-base/logging.h>
#include <health-impl/Health.h>
#include <health/utils.h>

#include <string_view>

#include "CapacityPathOverride.h"

#ifndef CHARGER_FORCE_NO_UI
#define CHARGER_FORCE_NO_UI 0
#endif

#if !CHARGER_FORCE_NO_UI
#include <health-impl/ChargerUtils.h>
#endif

using ::aidl::android::hardware::health::Health;
using ::aidl::android::hardware::health::r5x::OverrideHealthdConfigPaths;
using ::android::hardware::health::InitHealthdConfig;

#if !CHARGER_FORCE_NO_UI
using ::aidl::android::hardware::health::charger::ChargerCallback;
using ::aidl::android::hardware::health::charger::ChargerModeMain;
#endif

static constexpr const char* gInstanceName = "default";
static constexpr std::string_view gChargerArg{"--charger"};

int main(int argc, char** argv) {
#ifdef __ANDROID_RECOVERY__
    android::base::InitLogging(argv, android::base::KernelLogger);
#endif

    auto config = std::make_unique<healthd_config>();
    InitHealthdConfig(config.get());

    OverrideHealthdConfigPaths(config.get());

    auto binder = ndk::SharedRefBase::make<Health>(gInstanceName, std::move(config));

    if (argc >= 2 && argv[1] == gChargerArg) {
        android::base::InitLogging(argv, &android::base::KernelLogger);
#if !CHARGER_FORCE_NO_UI
        return ChargerModeMain(binder, std::make_shared<ChargerCallback>(binder));
#endif
    }

    auto hal_health_loop = std::make_shared<::aidl::android::hardware::health::HalHealthLoop>(
            binder, binder);
    return hal_health_loop->StartLoop();
}
