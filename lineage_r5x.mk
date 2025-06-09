#
# Copyright (C) 2022 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common Rising stuff
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Inherit from r5x device
$(call inherit-product, $(LOCAL_PATH)/device.mk)

PRODUCT_BRAND := realme
PRODUCT_DEVICE := r5x
PRODUCT_MANUFACTURER := realme
PRODUCT_NAME := lineage_r5x
PRODUCT_MODEL := realme 5 Series

PRODUCT_GMS_CLIENTID_BASE := android-oppo

# Inherit rising vendor configs
TARGET_BOOT_ANIMATION_RES := 1080
TARGET_ENABLE_BLUR := true
WITH_GMS := true
RISING_MAINTAINER := TrustedHacker
TARGET_DEFAULT_PIXEL_LAUNCHER := true
TARGET_INCLUDE_GOOGLE_DIALER := true

# Rising specific prop overrides
RISING_MAINTAINER="TrustedHacker"
PRODUCT_BUILD_PROP_OVERRIDES += \
    RisingChipset="Snapdragon_665™" \
    RisingMaintainer="TrustedHacker"

