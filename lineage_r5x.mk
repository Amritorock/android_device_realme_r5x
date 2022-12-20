#
# Copyright (C) 2022 Project LineageOs
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_p.mk)

# Inherit some common riceDroid stuff
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Inherit from r5x device
$(call inherit-product, $(LOCAL_PATH)/device.mk)

TARGET_GAPPS_ARCH := arm64
TARGET_INCLUDE_LIVE_WALLPAPERS := false

PRODUCT_BRAND := realme
PRODUCT_DEVICE := r5x
PRODUCT_MANUFACTURER := realme
PRODUCT_NAME := lineage_r5x
PRODUCT_MODEL := Realme 5 Series

PRODUCT_GMS_CLIENTID_BASE := android-realme

TARGET_VENDOR_PRODUCT_NAME := r5x
TARGET_VENDOR_DEVICE_NAME := r5x

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRODUCT_NAME="r5x" \
    PRIVATE_BUILD_DESC="trinket-user 10 QKQ1.200209.002 release-keys"

# Build info
BUILD_FINGERPRINT := "google/cheetah/cheetah:13/TQ1A.221205.011/9244662:user/release-keys"

PRODUCT_PROPERTY_OVERRIDES += \
ro.build.fingerprint=$(BUILD_FINGERPRINT)

# riceDroid Stuff
SUSHI_BOOTANIMATION  := 720
TARGET_ENABLE_BLUR := true
TARGET_SUPPORTS_GOOGLE_RECORDER := true
TARGET_BUILD_GRAPHENEOS_CAMERA := false
RICE_MAINTAINER := AMRITO
TARGET_USE_PIXEL_FINGERPRINT := true
TARGET_OPTOUT_GOOGLE_TELEPHONY := true
TARGET_BUILD_APERTURE_CAMERA := true
TARGET_EXCLUDES_AUDIOFX := false
