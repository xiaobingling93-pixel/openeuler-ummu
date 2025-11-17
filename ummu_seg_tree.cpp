// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */

#include <map>
#include <errno.h>
#include <stdlib.h>
#include "ummu_log.h"
#include "ummu_seg_tree.h"

struct Range {
	uint64_t left;
	uint64_t right;
	Range(uint64_t l, uint64_t size):left(l), right(l + size) {
	}
};

struct ModCmp {
	bool operator()(const Range& l, const Range& r) const {
		return l.right < r.left;
	}
};

using RangeTable = std::map<Range, void*, ModCmp>;

int ummu_create_seg_tree(uint64_t *output)
{
	*output = 0;
	auto table = new (std::nothrow)RangeTable();
	if (table == nullptr) {
		UMMU_MAPT_ERROR_LOG("Allocate range table memory failed.\n");
		return -ENOMEM;
	}

	*output = reinterpret_cast<uint64_t>(table);
	return 0;
}

int ummu_insert_seg(void *data, uint64_t seg_tree, uint64_t left, uint64_t size)
{
	auto table = reinterpret_cast<RangeTable *>(seg_tree);
	auto insert_ret = table->insert(std::make_pair(Range(left, size), data));
	if (!insert_ret.second) {
		UMMU_MAPT_ERROR_LOG("Insert range table failed.\n");
		return -EINVAL;
	}
	return 0;
}

void *ummu_find_seg(uint64_t left, uint64_t size, uint64_t seg_tree, int *comp)
{
	auto table = reinterpret_cast<RangeTable *>(seg_tree);
	auto find_ret = table->find(Range(left, size));
	if (find_ret != table->end()) {
		if ((find_ret->first.left == left) &&
			(find_ret->first.right == left + size)) {
			*comp = RANGE_MATCH;
			return find_ret->second;
		}
		*comp = RANGE_OVERLAP;
		return nullptr;
	}

	*comp = RANGE_NOT_EXIST;
	return nullptr;
}

int ummu_delete_seg(uint64_t left, uint64_t size, uint64_t seg_tree, clear_seg_node cleaner)
{
	auto table = reinterpret_cast<RangeTable *>(seg_tree);
	auto find_ret = table->find(Range(left, left + size));
	if ((find_ret != table->end()) &&
		(find_ret->first.left == left) &&
		(find_ret->first.right == (left + size))) {
		if (find_ret->second) {
			if (cleaner) {
				cleaner(find_ret->second);
			}
			free(find_ret->second);
		}
		table->erase(find_ret);
		return 0;
	}

	UMMU_MAPT_ERROR_LOG("Delete segment range not exists.\n");
	return -EINVAL;
}

void ummu_destroy_seg_tree(uint64_t *seg_tree, clear_seg_node cleaner)
{
	auto table = reinterpret_cast<RangeTable *>(*seg_tree);
	for (auto iter = table->begin(); iter != table->end();) {
		if (iter->second) {
			if (cleaner) {
				cleaner(iter->second);
			}
			free(iter->second);
		}
		iter = table->erase(iter);
	}

	delete table;
	table = nullptr;
	*seg_tree = 0;
}

