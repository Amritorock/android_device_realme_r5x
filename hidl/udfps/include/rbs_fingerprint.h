/*
 * Copyright (C) 2025 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <hardware/hw_auth_token.h>

typedef struct rbs_fingerprint_device {
    int (*rbs_initialize)(void* masterkey, uint32_t masterkey_size);
    int (*rbs_uninitialize)(void);
    int (*rbs_cancel)(void);
    int (*rbs_active_user_group)(uint32_t gid, const char* store_path);
    int (*rbs_set_data_path)();
    int (*rbs_chk_secure_id)(uint32_t gid, uint32_t user_id);
    int (*rbs_pre_enroll)(uint32_t gid, uint32_t seed);
    int (*rbs_enroll)(void);
    int (*rbs_post_enroll)(void);
    int (*rbs_chk_auth_token)(const hw_auth_token_t* hat, uint32_t hat_size);
    int (*rbs_authenticator)(uint32_t gid, void*, uint32_t, uint64_t operation_id);
    int (*rbs_remove_fingerprint)(uint32_t gid, uint32_t fid);
    int (*rbs_get_fingerprint_ids)(uint32_t gid, uint32_t* fids, uint32_t* num_fids);
    int (*rbs_get_authenticator_id)(uint64_t* authenticator_id);
    int (*rbs_set_on_callback_proc)(void* callback_proc);
    int (*rbs_extra_api)(uint32_t cmd, uint32_t* param, uint32_t param_size, uint32_t* param2,
                         uint32_t* param2_size);
#ifdef _HAS_QSEE
    int (*rbs_get_challenge)(uint64_t* challenge);
    int (*rbs_post_challenge)(uint64_t* challenge);
#endif
#ifdef _NEEDS_INI_RELOCATION
    char* g_custom_ini_path;
#endif
} rbs_fingerprint_device_t;
