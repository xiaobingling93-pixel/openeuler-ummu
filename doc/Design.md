# UMMU overview

               --------------------
               | user-mode process|
               --------------------
                        |
    --------------------------------------------------
    |               UMMU library                     |
    |    --------   ----------   ---------           |
    |    | main |   | segmng |   | queue |           |
    |    --------   ----------   ---------           |
    |    --------   ------------  ----------------   |
    |    | mapt |   | resource |  | common utils |   |
    |    --------   ------------  ----------------   |
    --------------------------------------------------
                  |                   |
        --------------------          |
        | UMMU-CORE driver |--------  |
        --------------------       |  |
               |                   |  |
        ---------------           -------
        | UMMU driver |           | DDR |
        ---------------           -------

Below is the summary for submodules in UMMU library:
1. __main:__ initialize UMMU context and open /dev/ummu/tid device.
2. __segmng:__ manage memory segments for every TID, including add or remove segments.
3. __queue:__ manage the creation and destruction of ummu user-mode queues in UMMU, and provide interfaces to
   send commands to these queues.
4. __mapt:__ manage MAPT(Memory Address Permission Table) for each TID, including insert,update, and clear operations.
5. __resource:__ interact with UMMU driver, including allocate/free TID, initialize/uninit user-mode queues ,
   allocate/free MAPT block etc.
6. __common utils:__ provide functions such as logging, memory mapping, bit manipulation, and other
general-purpose helpers.

# Introduction
The UMMU library manages serveral key resouces,including TID, MAPT, user-mode queues. The UMMU-CORE driver provides __ioctl__
interface, enabling user-mode processes to allocate and manage these resources through the device file /dev/ummu/tid.
  1. __tid:__ the UMMU-CORE driver allocates a unique token ID within a certain range, and return the TID.
  2. __MAPT:__ the MAPT is stored in DDR. The UMMU-CORE driver allocates physical memory and returns its virtual address to the
  user-mode process.The user-mode process manages MAPT entries-inserting, updating, or clearing them—by accessing the 
  virtual address with a specific offset.
  3. __user-mode queues:__ these queues are implemented in fixed hardware registers. The UMMU-CORE driver maps physical 
  address of the queue to a virtual address and return it to the user-mode process. The user-mode process sends commands by
  writing them to the mapped virtual address.

# Ristrictions
  1. When allocating a TID in MAPT_MODE_TABLE mode, the start VA must be 4K-aligned.
  2. Partial ungrant of a memory segment is not supported.
  3. Within a single TID, memory segments must not overlap.Overlapping is strictly prohibited.
  4. Since UMMU does not support IOPF, user-mode process must manually pin allocated memory pages before sharing them.
     Mmeory pin unctioins(e.g. pin_user_pages_fast) can only be called from kernel space.Therefore, a dedicated kernel
     driver is typically used to manage memory allocation and pinning. The driver allocates memory, pins it, and returns
     a mapped virtual address via mmap to the user-mode process.

