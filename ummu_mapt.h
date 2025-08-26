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

#define MAX_LEVEL_INDEX 3U

struct ummu_mapt_queue_ctx {
        uint32_t ummu_cnt;
        size_t queue_size;
        uint16_t cmd_que_pi_size;
        uint16_t cpl_que_size;

        struct ummu_mapt_queue_args *queue;
};

struct ummu_mapt_info {
        unsigned int tid;
        uint8_t double_plbi;
        uint8_t kcmd_plbi;
        enum ummu_mapt_mode mode;

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

struct ummu_data_info {
        void *data;
        size_t data_size;
        uint64_t data_base;
        struct ummu_mapt_info *mapt_info;
};

#define MAX_CMD_QUE_INDEX(size) (((uint64_t)1 << (size)) - (uint64_t)1)

/* =========================================== common global variable ============================================ */
extern struct ummu_ctx_info *get_ummu_ctx(void);

#endif
