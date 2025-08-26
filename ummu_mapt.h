/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */

#ifndef _UMMU_MAPT_H_
#define _UMMU_MAPT_H_

#include <stdint.h>
#include <pthread.h>
#include "ummu_api.h"

/* =============================================== common data structure ======================================== */
struct ummu_mapt_entry_node {
        uint32_t valid : 1;
        uint32_t reserved_0 : 2;
        uint32_t e_bit : 1;
        uint32_t permission : 6;
        uint32_t reserved_1 : 22;

        uint32_t reserved_2;

        uint32_t base_low;
        uint32_t base_high : 16;
        uint32_t reserved_3 : 12;
        uint32_t reserved_4 : 3;
        uint32_t token_check : 1;

        uint32_t limit_low;
        uint32_t limit_high : 16;
        uint32_t reserved_5 : 12;
        uint32_t reserved_6 : 2;
        uint32_t nonce : 2;

        uint32_t token_val_0;
        uint32_t token_val_1;
};

#define MAX_LEVEL_INDEX 3U

#define INDEX_MAX_SIZE 512UL

#define UMMU_MAX_TOKEN_NUM 2

struct ummu_mapt_table_ctx {
        uint64_t granted_addr_mng;
};

struct ummu_mapt_queue_ctx {
        uint32_t ummu_cnt;
        size_t queue_size;
        uint16_t cmd_que_pi_size;
        uint16_t cpl_que_size;

        struct ummu_mapt_queue_args *queue;
};

struct ummu_seg_info {
        uint64_t start_addr;
        uint64_t grant_size;
        enum ummu_mapt_perm permission;
        enum ummu_ebit_state e_bit;
        /* true: check token  false: not check token */
        uint8_t token_check;
        uint8_t token_count;
        uint32_t token_val[UMMU_MAX_TOKEN_NUM];
};

struct ummu_mapt_info {
        union {
                struct ummu_mapt_entry_node *entry_block;
                struct ummu_mapt_table_ctx *table_ctx;
        } block_base;

        unsigned int tid;
        uint16_t valid;
        uint8_t double_plbi;
        uint8_t kcmd_plbi;
        enum ummu_mapt_mode mode;
        /* Notice: when using mapt_mutex, you must ensure that g_ummu_ctx->ctx_mutex is already locked. */
        pthread_mutex_t mapt_mutex;

        struct ummu_mapt_queue_ctx queue_ctx;
};

struct ummu_ctx_info {
        uint32_t tid_cnt;
        pthread_mutex_t ctx_mutex;
        void *mapt_map;
        int shared_fd;
};

enum ummu_buf_mode {
        BASE_MODE_ENTRY_BLOCK,
        BASE_MODE_QUEUE
};

enum ummu_grant_op_type {
        UMMU_GRANT = 0,
        UMMU_ADD_TOKEN = 1,
        UMMU_REMOVE_TOKEN = 2,
        UMMU_UNGRANT = 3,
        UMMU_OP_END
};

struct ummu_data_info {
        void *data;
        size_t data_size;
        enum ummu_mapt_perm perm;
        struct ummu_token_info *token;
        uint32_t tokenval;
        uint64_t data_base;
        uint64_t data_limit;
        uint32_t token_check;
        int bytoken;
        enum ummu_ebit_state e_bit;
        enum ummu_grant_op_type op;
        struct ummu_mapt_info *mapt_info;
        uint8_t lvl;
};

#define MAX_CMD_QUE_INDEX(size) (((uint64_t)1 << (size)) - (uint64_t)1)

#define ADDR_FULL(low, high) (((uint64_t)(high) << 32UL) | (uint64_t)(low))

/* =========================================== common global variable ============================================ */
extern struct ummu_ctx_info *get_ummu_ctx(void);

#endif
