#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixup_remove,
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/realme/r5x',
    'hardware/qcom-caf/sm8150',
    'hardware/qcom-caf/wlan',
    'vendor/qcom/opensource/commonsys/display',
    'vendor/qcom/opensource/commonsys-intf/display',
    'vendor/qcom/opensource/dataservices',
    'vendor/qcom/opensource/data-ipa-cfg-mgr-legacy-um',
    'vendor/qcom/opensource/display',
]


def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'vendor' else None


lib_fixups: lib_fixups_user_type = {
    **lib_fixups,
    (
        'com.qualcomm.qti.dpm.api@1.0',
        'vendor.qti.hardware.fm@1.0',
        'vendor/lib64/vendor.qti.data.factory@2.0.so',
        'vendor/lib64/vendor.qti.data.factory@2.1.so',
        'vendor/lib64/vendor.qti.data.factory@2.2.so',
        'vendor/lib64/vendor.qti.data.factory@2.3.so',
        'vendor/lib64/vendor.qti.data.mwqem@1.0.so',
        'vendor/lib64/vendor.qti.data.slm@1.0.so',
        'vendor/lib64/vendor.qti.hardware.data.cne.internal.api@1.0.so',
        'vendor/lib64/vendor.qti.hardware.data.cne.internal.constants@1.0.so',
        'vendor/lib64/vendor.qti.hardware.data.cne.internal.server@1.0.so',
        'vendor/lib64/vendor.qti.hardware.data.connection@1.0.so',
        'vendor/lib64/vendor.qti.hardware.data.connection@1.1.so',
        'vendor/lib64/vendor.qti.hardware.data.dynamicdds@1.0.so',
        'vendor/lib64/vendor.qti.hardware.data.dynamicdds@1.1.so',
        'vendor/lib64/vendor.qti.hardware.data.flow@1.0.so',
        'vendor/lib64/vendor.qti.hardware.data.iwlan@1.0.so',
        'vendor/lib64/vendor.qti.hardware.data.latency@1.0.so',
        'vendor/lib64/vendor.qti.hardware.data.lce@1.0.so',
        'vendor/lib64/vendor.qti.hardware.data.qmi@1.0.so',
        'vendor/lib64/vendor.qti.hardware.slmadapter@1.0.so',
        'vendor/lib64/vendor.qti.latency@2.0.so',
        'vendor/lib64/vendor.qti.latency@2.1.so',
        'vendor/lib/com.qualcomm.qti.dpm.api@1.0.so',
        'vendor/lib64/com.qualcomm.qti.dpm.api@1.0.so',
        'vendor/lib/vendor.qti.hardware.mwqemadapter@1.0.so',
        'vendor/lib64/vendor.qti.hardware.mwqemadapter@1.0.so',
        'vendor/lib64/vendor.qti.hardware.radio.am@1.0.so',
        'vendor/lib64/vendor.qti.hardware.radio.lpa@1.0.so',
        'vendor/lib64/vendor.qti.hardware.radio.lpa@1.1.so',
        'vendor/lib64/vendor.qti.hardware.radio.lpa@1.2.so',
        'vendor/lib64/vendor.qti.hardware.radio.qtiradio@1.0.so',
        'vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.0.so',
        'vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.1.so',
        'vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.2.so',
        'vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.3.so',
        'vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.4.so',
        'vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.5.so',
        'vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.6.so',
        'vendor/lib64/vendor.qti.hardware.radio.qtiradio@2.7.so',
        'vendor/lib64/vendor.qti.hardware.radio.uim@1.0.so',
        'vendor/lib64/vendor.qti.hardware.radio.uim@1.1.so',
        'vendor/lib64/vendor.qti.hardware.radio.uim@1.2.so',
        'vendor/lib64/vendor.qti.hardware.radio.uim_remote_client@1.0.so',
        'vendor/lib64/vendor.qti.hardware.radio.uim_remote_client@1.1.so',
        'vendor/lib64/vendor.qti.hardware.radio.uim_remote_client@1.2.so',
        'vendor/lib64/vendor.qti.hardware.radio.uim_remote_server@1.0.so',
        'vendor/lib64/com.qualcomm.qti.imscmservice@2.0.so',
        'vendor/lib64/com.qualcomm.qti.imscmservice@2.1.so',
        'vendor/lib64/com.qualcomm.qti.imscmservice@2.2.so',
        'vendor/lib64/com.qualcomm.qti.uceservice@2.0.so',
        'vendor/lib64/com.qualcomm.qti.uceservice@2.1.so',
        'vendor/lib64/com.qualcomm.qti.uceservice@2.2.so',
        'vendor/lib64/com.qualcomm.qti.uceservice@2.3.so',
        'vendor/lib64/vendor.qti.hardware.radio.ims@1.0.so',
        'vendor/lib64/vendor.qti.hardware.radio.ims@1.1.so',
        'vendor/lib64/vendor.qti.hardware.radio.ims@1.2.so',
        'vendor/lib64/vendor.qti.hardware.radio.ims@1.3.so',
        'vendor/lib64/vendor.qti.hardware.radio.ims@1.4.so',
        'vendor/lib64/vendor.qti.hardware.radio.ims@1.5.so',
        'vendor/lib64/vendor.qti.hardware.radio.ims@1.6.so',
        'vendor/lib64/vendor.qti.hardware.radio.ims@1.7.so',
        'vendor/lib64/vendor.qti.hardware.radio.ims@1.8.so',
        'vendor/lib64/vendor.qti.ims.callcapability@1.0.so',
        'vendor/lib64/vendor.qti.ims.callinfo@1.0.so',
        'vendor/lib64/vendor.qti.ims.factory@1.0.so',
        'vendor/lib64/vendor.qti.ims.factory@1.1.so',
        'vendor/lib64/vendor.qti.ims.rcsconfig@1.0.so',
        'vendor/lib64/vendor.qti.ims.rcsconfig@1.1.so',
        'vendor/lib64/vendor.qti.ims.rcsconfig@2.0.so',
        'vendor/lib64/vendor.qti.ims.rcsconfig@2.1.so',
        'vendor/lib64/vendor.qti.imsrtpservice@3.0.so',
    ): lib_fixup_vendor_suffix,
    'libwpa_client': lib_fixup_remove,
}

blob_fixups: blob_fixups_user_type = {
    ('vendor/lib/libOPPORectify.so', 'vendor/lib/libarcsoft_beautyshot_lite_image.so', 'vendor/lib/libarcsoft_hdr_couple_api.so', 'vendor/lib/libarcsoft_high_dynamic_range_couple.so', 'vendor/lib/libarcsoft_picauto.so', 'vendor/lib/libblur_channel.so', 'vendor/lib/libthread_blur.so', 'vendor/lib/libdepthmap.so'): blob_fixup()
        .replace_needed('libstdc++.so', 'libstdc++_vendor.so'),
    'vendor/lib64/libwvhidl.so': blob_fixup()
        .add_needed('libcrypto_shim.so'),
    ('vendor/etc/init/android.hardware.drm@1.3-service.widevine.rc', 'vendor/etc/init/vendor.qti.media.c2@1.0-service.rc'): blob_fixup()
        .regex_replace(r'writepid /dev/cpuset/foreground/tasks', 'task_profiles ProcessCapacityHigh'),
    'vendor/lib64/libdpps.so': blob_fixup()
        .replace_needed('libtinyxml2.so', 'libtinyxml2-v34.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'r5x',
    'realme',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
