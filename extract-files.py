#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.file import File
from extract_utils.fixups_blob import (
    BlobFixupCtx,
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
from extract_utils.tools import (
    llvm_objdump_path,
)
from extract_utils.utils import (
    run_cmd,
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
