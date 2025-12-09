# SPDX-License-Identifier: MIT
# Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.

cmake_minimum_required(VERSION 3.5)
project(UMMU_SO VERSION ${PROJECT_VERSION} LANGUAGES C CXX)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} UMMU_DIR_SRCS)

add_library(UMMU_SO SHARED ${UMMU_DIR_SRCS})

target_include_directories(UMMU_SO PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/kernel_headers)

set(UMMU_NAME ummu)
set_target_properties(UMMU_SO PROPERTIES OUTPUT_NAME ${UMMU_NAME}  
        SOVERSION ${LIB_SOVERSION}
        VERSION ${PROJECT_VERSION}
        )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wuninitialized -Wno-error=format -Wundef -Wunused -Wdate-time -Wfloat-equal -Wshadow -Wvla \
        -Wdisabled-optimization -Wempty-body -Wignored-qualifiers -Wimplicit-fallthrough=3 -Wtype-limits -Wshift-negative-value -Wswitch-default \
        -Wframe-larger-than=8192 -Wshift-overflow=2 -Wwrite-strings -Wmissing-format-attribute -Wformat-nonliteral -Wduplicated-cond -Wtrampolines \
        -Wlogical-op -Wsuggest-attribute=format -Wduplicated-branches -Wmissing-include-dirs -Wformat-signedness -Wmissing-declarations \
        -Wreturn-local-addr -Wredundant-decls -Wfloat-conversion -Wmissing-prototypes -Wstrict-prototypes -Werror -Wextra -fno-common -std=gnu99")

target_compile_options(UMMU_SO PRIVATE
    -fstack-protector-all
)

target_link_options(UMMU_SO PRIVATE
    -Wl,-z,relro,-z,now
)

install(TARGETS UMMU_SO LIBRARY DESTINATION lib64 OPTIONAL)
install(FILES include/ummu_api.h DESTINATION include)
install(FILES kernel_headers/ummu_core.h DESTINATION include)
