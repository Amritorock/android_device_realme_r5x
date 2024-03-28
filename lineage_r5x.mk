#
# Copyright (C) 2022 The Project Elixir
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common LineageOS stuff
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Inherit from r5x device
$(call inherit-product, $(LOCAL_PATH)/device.mk)

TARGET_DISABLE_EPPE := true
TARGET_BOOT_ANIMATION_RES := 720
TARGET_GAPPS_ARCH := arm64
TARGET_INCLUDE_LIVE_WALLPAPERS := false

PRODUCT_BRAND := realme
PRODUCT_DEVICE := r5x
PRODUCT_MANUFACTURER := realme
PRODUCT_NAME := lineage_r5x
PRODUCT_MODEL := Realme 5 Series

PRODUCT_GMS_CLIENTID_BASE := android-oppo
