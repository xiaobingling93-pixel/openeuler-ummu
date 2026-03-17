# Welcome to use UMMU library
  UMMU library makes the memory sharement between user-mode process and I/O device available.
# Overview
  The UMMU library toolkit includes one shared library libummu. It supports two main functions:
   - Allocate unique token id for user-mode process and I/O device
   - Manage the access rights for shared memory
  One token id can be bond with multple memory segments, and it must be released once no longer in use.

# Build and Install
  ## For Production Use
  Two rpm packages are supported: libummu-${version}.aarch64.rpm, libummu-devel-${version}.aarch64.rpm. You can download at (xxx).
   You can install them by using:

    rpm -ivh libummu-${version}.aarch64.rpm
    rpm -ivh libummu-devel-${version}.aarch64.rpm

   libummu.so would be installed at /usr/lib64, include files would be installed at /usr/include.

  ## For Testing and Development
   A local copy of the Git Repository can be obtained by cloning it from the original UMMU library repository using:

    git clone https://atomgit.com/openeuler/ummu

# License
  UMMU library is licensed under SPDX-License-Identifier: MIT.

# Copyright
  Copyright (c) 2025 HiSilicon Technologies Co., Ltd.
  All rights reserved.
