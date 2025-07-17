#define NPROC        64  // maximum number of processes
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       1000  // size of file system in blocks
#define MAXPATH      128   // maximum file path name

// Protection flags
#define PROT_NONE       0x0     /* no permissions */
#define PROT_READ       0x1     /* pages can be read */
#define PROT_WRITE      0x2     /* pages can be written */
#define PROT_EXEC       0x4     /* pages can be executed */ // <-- 添加这一行

// Flags for mmap
#define MAP_SHARED      0x01    /* Share changes */
#define MAP_PRIVATE     0x02    /* Changes are private */
#define MAP_ANONYMOUS   0x04    /* Don't use a file */
#define MAP_FIXED       0x10    /* Interpret addr exactly */