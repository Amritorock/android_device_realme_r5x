/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "rbs_fingerprint.h"

class UdfpsHandler {
  public:
    virtual ~UdfpsHandler() = default;

    virtual void init(rbs_fingerprint_device_t* device) = 0;
    virtual void onFingerDown(uint32_t x, uint32_t y, float minor, float major) = 0;
    virtual void onFingerUp() = 0;

    virtual void onAcquired(int32_t result, int32_t vendorCode) = 0;
    virtual void cancel() = 0;
};

struct UdfpsHandlerFactory {
    UdfpsHandler* (*create)();
    void (*destroy)(UdfpsHandler* handler);
};

UdfpsHandlerFactory* getUdfpsHandlerFactory();
