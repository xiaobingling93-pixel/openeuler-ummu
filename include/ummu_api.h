/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */

#ifndef _UMMU_API_H_
#define _UMMU_API_H_

#include <unistd.h>
#include <ummu_core.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int ummu_allocate_tid(struct ummu_tid_attr *tid_attr, uint32_t *tid);

int ummu_grant(uint32_t tid, void *data, size_t data_size,
        enum ummu_mapt_perm perm, struct ummu_seg_attr *seg_attr);

int ummu_free_tid(uint32_t tid);
#ifdef __cplusplus
}
#endif

#endif
