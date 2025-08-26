// SPDX-License-Identifier: MIT
/*
* Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "ummu_map.h"
#include "ummu_resource.h"
#include "ummu_queue.h"
#include "ummu_api.h"
#include "ummu_log.h"
#include "ummu_level_block.h"
#include "ummu_common.h"
#include "ummu_seg_mng.h"
#include "ummu_mapt.h"

static int ummu_mapt_entry_init(struct ummu_mapt_info *mapt_info)
{
        struct ummu_mapt_entry_node *entry_node;

        entry_node = (struct ummu_mapt_entry_node *)ummu_get_core_buf(mapt_info->tid,
                BASE_MODE_ENTRY_BLOCK, BLOCK_SIZE_4K, get_ummu_ctx()->shared_fd, 0);
        if (CORE_BUF_CHECK_INVALID(entry_node)) {
                UMMU_MAPT_ERROR_LOG("Entry node is invalid.\n");
                return -ENOMEM;
        }
        entry_node->valid = 0;
        entry_node->nonce = 0;
        mapt_info->block_base.entry_block = entry_node;

        return 0;
}

static int ummu_mapt_table_ctx_init(struct ummu_mapt_info *mapt_info, struct ummu_tid_info *info)
{
        struct ummu_mapt_table_ctx *table_ctx;
        struct ummu_mapt_block *block = NULL;
        struct ummu_mapt_table_node pre_node;
        struct ummu_mapt_table_node *root;
        int ret;

        table_ctx = (struct ummu_mapt_table_ctx *)malloc(sizeof(struct ummu_mapt_table_ctx));
        if (table_ctx == NULL) {
                UMMU_MAPT_ERROR_LOG("Alloc table_ctx memory failed.\n");
                ret = -ENOMEM;
                goto err_malloc_table_ctx;
        }
        (void)memset(table_ctx, 0, sizeof(struct ummu_mapt_table_ctx));

        ret = ummu_create_seg_mng(&table_ctx->granted_addr_mng);
        if (ret != 0) {
                UMMU_MAPT_ERROR_LOG("Init granted addr manager failed.\n");
                ret = -ENOMEM;
                goto err_create_rbtree;
        }

        mapt_info->block_base.table_ctx = table_ctx;
        mapt_info->block_base.table_ctx->expan = info->hw_cap & HW_CAP_EXPAN;
        mapt_info->block_base.table_ctx->blk_exp_size = info->blk_exp_size;

        root = ummu_alloc_level_block(mapt_info, &pre_node, block);
        if (root == NULL) {
                UMMU_MAPT_ERROR_LOG("Alloc level block failed.\n");
                ret = -errno;
                goto err_alloc_level_block;
        }

        return 0;
err_alloc_level_block:
        ummu_destroy_seg_mng(&table_ctx->granted_addr_mng);
err_create_rbtree:
        free(table_ctx);
err_malloc_table_ctx:
        table_ctx = NULL;
        return ret;
}

static struct ummu_mapt_info *ummu_alloc_mapt_mem(struct ummu_tid_info *info)
{
        struct ummu_mapt_info *mapt_info;

        mapt_info = (struct ummu_mapt_info *)malloc(sizeof(struct ummu_mapt_info));
        if (mapt_info == NULL) {
                UMMU_MAPT_ERROR_LOG("Alloc mapt info memory failed.\n");
                return NULL;
        }
        (void)memset(mapt_info, 0, sizeof(struct ummu_mapt_info));

        (void)pthread_mutex_init(&mapt_info->mapt_mutex, NULL);
        mapt_info->mode = info->mode;
        mapt_info->tid = info->tid;
        mapt_info->valid = 1;
        mapt_info->double_plbi = info->hw_cap & HW_CAP_DOUBLE_PLBI;
        mapt_info->kcmd_plbi = info->hw_cap & HW_CAP_KCMD_PLBI;

        return mapt_info;
}

static struct ummu_mapt_info *ummu_mapt_create(struct ummu_tid_info *info)
{
        struct ummu_mapt_info *mapt_info;
        int ret;

        mapt_info = ummu_alloc_mapt_mem(info);
        if (mapt_info == NULL) {
                goto err_alloc_mapt_info;
        }

        ret = ummu_queue_create(mapt_info, info, get_ummu_ctx()->shared_fd);
        if (ret != 0) {
                UMMU_MAPT_ERROR_LOG("Alloc plb resource failed.\n");
                goto err_queue_create;
        }

        if (mapt_info->mode == MAPT_MODE_ENTRY) {
                ret = ummu_mapt_entry_init(mapt_info);
        } else {
                ret = ummu_mapt_table_ctx_init(mapt_info, info);
        }
        if (ret != 0) {
                goto err_mapt_init;
        }

        return mapt_info;

err_mapt_init:
        ummu_queue_destroy(mapt_info);
err_queue_create:
        free(mapt_info);
err_alloc_mapt_info:
        return NULL;
}

static int ummu_check_mode(enum ummu_mapt_mode mode)
{
        if (mode >= MAPT_MODE_END || mode < MAPT_MODE_TABLE) {
                UMMU_MAPT_ERROR_LOG("Perm table mode[%u] is invalid.\n", mode);
                return -EINVAL;
        }

        return 0;
}

static int ummu_check_input_param(enum ummu_mapt_mode mode, unsigned int *tid)
{
        int ret;

        ret = ummu_check_mode(mode);
        if (ret != 0) {
                return ret;
        }

        if (tid == NULL) {
                UMMU_MAPT_ERROR_LOG("Tid is null.\n");
                return -EINVAL;
        }

        return 0;
}

static void ummu_mapt_table_ctx_uninit(struct ummu_mapt_info *mapt_info)
{
        struct ummu_mapt_table_ctx *table_ctx;
        struct ummu_mapt_block *block;
        unsigned long idx, block_id;

        table_ctx = mapt_info->block_base.table_ctx;

        if (table_ctx->granted_addr_mng != 0) {
                ummu_destroy_seg_mng(&table_ctx->granted_addr_mng);
        }

        for_each_set_bit(idx, table_ctx->level_block_bitmap, MAX_LEVEL_ID_SIZE) {
                block_id = idx / PER_MAPT_LEVEL_BLOCK_CNT;
                if ((idx % PER_MAPT_LEVEL_BLOCK_CNT) != 0) {
                        continue;
                }
                block = (struct ummu_mapt_block *)table_ctx->mapt_block_array[block_id];
                table_ctx->mapt_block_array[block_id] = NULL;
                if (block == NULL) {
                        UMMU_MAPT_ERROR_LOG("Free block id = %lu failed.\n", block_id);
                        continue;
                }
                ummu_free_core_buf(BASE_MODE_TABLE_BLOCK, (void *)block->block_addr, table_ctx->blk_exp_size);
                free(block);
                block = NULL;
        }

        if (mapt_info->block_base.table_ctx != NULL) {
                free(mapt_info->block_base.table_ctx);
                mapt_info->block_base.table_ctx = NULL;
        }
}

static void ummu_mapt_entry_uninit(struct ummu_mapt_info *mapt_info)
{
        ummu_free_core_buf(BASE_MODE_ENTRY_BLOCK, (void *)mapt_info->block_base.entry_block, 0);
}

void ummu_mapt_destroy(struct ummu_mapt_info *info)
{
        if (info->mode == MAPT_MODE_ENTRY) {
                ummu_mapt_entry_uninit(info);
        } else if (info->mode == MAPT_MODE_TABLE) {
                ummu_mapt_table_ctx_uninit(info);
        } else {
                UMMU_MAPT_ERROR_LOG("Mode is invalid, only support entry or table.\n");
                return;
        }

        ummu_queue_destroy(info);
        free(info);
}

int ummu_free_tid(uint32_t tid)
{
        struct ummu_ctx_info *ummu_ctx = get_ummu_ctx();
        struct ummu_mapt_info *mapt_info;

        if (ummu_ctx == NULL) {
                UMMU_MAPT_ERROR_LOG("Ummu ctx is null.\n");
                return -EINVAL;
        }

        (void)pthread_mutex_lock(&ummu_ctx->ctx_mutex);
        mapt_info = (struct ummu_mapt_info *)ummu_map_get(ummu_ctx->mapt_map, tid);
        if (mapt_info == NULL || mapt_info->valid == 0) {
                (void)pthread_mutex_unlock(&ummu_ctx->ctx_mutex);
                return -EINVAL;
        }

        (void)pthread_mutex_lock(&mapt_info->mapt_mutex);
        ummu_ctx->tid_cnt--;
        (void)ummu_map_del(ummu_ctx->mapt_map, tid);
        (void)pthread_mutex_unlock(&ummu_ctx->ctx_mutex);
        (void)pthread_mutex_unlock(&mapt_info->mapt_mutex);
        ummu_mapt_destroy(mapt_info);
        ummu_put_tid(ummu_ctx->shared_fd, tid);

        return 0;
}

int ummu_allocate_tid(struct ummu_tid_attr *tid_attr, uint32_t *tid)
{
        struct ummu_tid_info info = {};
        struct ummu_ctx_info *ummu_ctx = get_ummu_ctx();
        struct ummu_mapt_info *mapt_info;
        int ret;

        if (ummu_ctx == NULL || tid_attr == NULL) {
                UMMU_MAPT_ERROR_LOG("UMMU context %s NULL, tid_attr %s NULL.\n",
                        ummu_ctx == NULL ? "is" : "not", tid_attr == NULL ? "is" : "not");
                return -EINVAL;
        }

        ret = ummu_check_input_param(tid_attr->mode, tid);
        if (ret != 0) {
                return ret;
        }
        info.mode = tid_attr->mode;

        ret = ummu_get_tid(ummu_ctx->shared_fd, &info);
        if (ret != 0) {
                UMMU_MAPT_ERROR_LOG("Get tid failed, ret = %d.\n", ret);
                return ret;
        }

        mapt_info = ummu_mapt_create(&info);
        if (mapt_info == NULL) {
                UMMU_MAPT_ERROR_LOG("Create ummu mapt failed.\n");
                ret = -ENOMEM;
                goto err_mapt_create;
        }

        (void)pthread_mutex_lock(&ummu_ctx->ctx_mutex);
        ret = ummu_map_insert(ummu_ctx->mapt_map, info.tid, (void *)mapt_info);
        if (ret != 0) {
                UMMU_MAPT_ERROR_LOG("Insert mapt map failed.\n");
                goto err_map_insert;
        }
        ummu_ctx->tid_cnt++;
        (void)pthread_mutex_unlock(&ummu_ctx->ctx_mutex);
        *tid = info.tid;

        return 0;

err_map_insert:
        (void)pthread_mutex_unlock(&ummu_ctx->ctx_mutex);
        ummu_mapt_destroy(mapt_info);
err_mapt_create:
        ummu_put_tid(ummu_ctx->shared_fd, info.tid);
        return ret;
}