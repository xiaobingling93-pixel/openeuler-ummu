// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */

#include <cstdint>
#include <cstdlib>
#include <unordered_map>
#include "ummu_utils.h"

using namespace std;

void *create_map(void)
{
        unordered_map<uint32_t, void *> *map = new unordered_map<uint32_t, void *>();
        return static_cast<void *>(map);
}

int hashmap_insert(void *map, uint32_t key, void *value)
{
        unordered_map<uint32_t, void *> *map_ptr = static_cast<unordered_map<uint32_t, void *> *>(map);
        auto result = map_ptr->find(key);
        if (result == map_ptr->end()) {
                map_ptr->emplace(key, value);
                return 0;
        } else {
                return -1;
        }
}

void *hashmap_get(void *map, uint32_t key)
{
        void *value = nullptr;
        unordered_map<uint32_t, void *> *map_ptr = static_cast<unordered_map<uint32_t, void *> *>(map);
        auto result = map_ptr->find(key);
        if (result != map_ptr->end()) {
                value = result->second;
        }

        return value;
}

size_t hashmap_del(void *map, uint32_t key)
{
        unordered_map<uint32_t, void *> *map_ptr = static_cast<unordered_map<uint32_t, void *> *>(map);
        return map_ptr->erase(key);
}

void hashmap_clear(void *map, value_destroy destroyer)
{
        unordered_map<uint32_t, void *> *map_ptr = static_cast<unordered_map<uint32_t, void *> *>(map);
        for (auto it = map_ptr->begin(); it != map_ptr->end(); it++) {
                if (destroyer) {
                        destroyer(static_cast<void *>(it->second));
                } else {
                        free(it->second);
                }
        }
        map_ptr->clear();
        delete map_ptr;
}