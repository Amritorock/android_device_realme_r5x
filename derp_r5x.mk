#
# Copyright (C) 2022 The Project Elixir
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common Project-Elixir stuff
$(call inherit-product, vendor/derp/config/common_full_phone.mk)

# Inherit from r5x device
$(call inherit-product, $(LOCAL_PATH)/device.mk)

TARGET_BOOT_ANIMATION_RES := 720
TARGET_GAPPS_ARCH := arm64
TARGET_INCLUDE_LIVE_WALLPAPERS := false
TARGET_NOT_USES_BLUR := true
TARGET_USES_MINI_GAPPS := true
USE_LEGACY_BOOTANIMATION := true

PRODUCT_BRAND := realme
PRODUCT_DEVICE := r5x
PRODUCT_MANUFACTURER := realme
PRODUCT_NAME := derp_r5x
PRODUCT_MODEL := Realme 5 Serie

PRODUCT_GMS_CLIENTID_BASE := android-oppo

TARGET_VENDOR_PRODUCT_NAME := r5x
TARGET_VENDOR_DEVICE_NAME := r5x

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRODUCT_NAME="r5x" \
    PRIVATE_BUILD_DESC="trinket-user 10 QKQ1.200209.002 release-keys"
