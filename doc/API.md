# ummu_allocate_tid(struct ummu_tid_attr \*tid_attr, uint32_t \*tid)
  Allocate unique ID for user-mode process and I/O device.
  - parameters
    - input parameter: tid_attr
    - output parameter: tid
    - return value:
      0: success
      others: failed
  - enum ummu_mapt_mode: Memory Address Permission Table mode
    - MAPT_MODE_ENTRY: Only support one memory address segment management for one tid
    - MAPT_MODE_TABLE: Support multiple memory address segments manamgement for one tid
  - struct ummu_tid_attr
  	- enum ummu_mapt_mode mode

# ummu_grant(uint32_t tid, void \*data, size_t data_size,enum ummu_mapt_perm perm, struct ummu_seg_attr \*seg_attr)
  Add access permissions for one memory segment, and bind this memory segment with one tid.
  - parameters
    - input parameters:
      - tid: allocated by interface ummu_allocate_tid
      - data: the start of virtual address
      - size: the memory size intended to be granted
      - perm: access rights
      - seg_attr: other attributes for target memory segments
    - return value:
      0: success
      others: failed
  - enum ummu_mapt_perm: access permissions for shared-memory segment
    - MAPT_PERM_W: write only
    - MAPT_PERM_R: read only
    - MAPT_PERM_RW: read and write
    - MAPT_PERM_ATOMIC_W: atomic write only
    - MAPT_PERM_ATOMIC_R: atomic read only
    - MAPT_PERM_ATOMIC_RW: atoic read and write
  - struct ummu_seg_attr
    - struct ummu_token_info
      - unsigned int input: Only 0 and 1 are allowed.if input is 0, input tokenVal would be used; else ummu library would         generate token value.
      - unsigned int tokenVal: When input is 1, this would be used.
    - enum ummu_ebit_state
      - UMMU_EBIT_OFF: enable ebit check
      - UMMU_EBIT_ON: disable ebit check

# ummu_ungrant(uint32_t tid, void \*data, size_t size)
  Remove access permissions for one memory segment, and unbind memory segment with tid.
  - parameters
    - input parameters:
      - tid: allocated by interface ummu_allocate_tid
      - data: the start of virtual address
      - size: the memory size intended to be ungranted
    - return value:
      0: success
      others: failed

# ummu_ungrant_by_token(uint32_t tid, void \*data, size_t size, uint32_t token_val)
  Remove one token value from the permssion table of target memory segment.
  - parameters
    - input parameters:
      - tid: allocated by interface ummu_allocate_tid
      - data: the start of virtual address
      - size: the memory size intended to be ungranted
      - taken_val: intend to be removed token value
    - return value:
      0: success
      others: failed

# ummu_free_tid(uint32_t tid)
  Free all memory segments resources bond with target tid.
  - parameters
    - input parameters:
      - tid: intend to free
    - return value:
      0: success
      others: failed

# Demos
  Below is a C demo. It shows how the user-mode process allocates memory and shares with I/O device.

	struct ummu_tid_attr tid_attr = {.mode = MAPT_MODE_TABLE};
    struct dev_dma_info info = { 0 };
    struct ummu_token_info token = {
        .tokenVal = 0xbeaf,
    };
    struct ummu_seg_attr seg_attr = {.token = &token, .e_bit = UMMU_EBIT_OFF};
    info.token_info.tokenVal = token.tokenVal;
    info.mode = MAPT_MODE_TABLE;
    info.with_token = 1;
    info.size = 0x1000;

    info->va = (uint32_t *)mmap(NULL, info.size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (!info->va) {
		printf("mmap memory %d bytes failed.\n", info->size);
		return -1;
	}

	ummu_allocate_tid(&tid_addr, &info.tid);
    ummu_grant(info.tid, info.va, info.size, MAPT_PERM_R, &seg_attr);
    ummu_ungrant(info.tid, info.va, info.size);
    ummu_free_tid(info.tid);

    munmap(info.va, info.size);

