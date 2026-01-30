#
# Copyright (C) 2022 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common AxionAOSP stuff
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Inherit from r5x device
$(call inherit-product, $(LOCAL_PATH)/device.mk)

PRODUCT_BRAND := realme
PRODUCT_DEVICE := r5x
PRODUCT_MANUFACTURER := realme
PRODUCT_NAME := lineage_r5x
PRODUCT_MODEL := realme 5 Series

PRODUCT_GMS_CLIENTID_BASE := android-oppo

# AxionAOSP stuff
AXION_CAMERA_REAR_INFO := 12,8,2,2
AXION_CAMERA_FRONT_INFO := 8
AXION_MAINTAINER := Amrito_Karmokar
AXION_PROCESSOR := Snapdragon_665™
TARGET_ENABLE_BLUR := true
TORCH_STR_SUPPORTED := true
PERF_GOV_SUPPORTED := true
PERF_DEFAULT_GOV := schedutil
GPU_FREQS_PATH := /sys/class/kgsl/kgsl-3d0/devfreq/available_frequencies
GPU_MIN_FREQ_PATH := /sys/class/kgsl/kgsl-3d0/devfreq/min_freq
