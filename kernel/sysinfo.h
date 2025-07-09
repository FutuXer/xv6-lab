<<<<<<< HEAD
struct sysinfo {
  uint64 freemem;   // amount of free memory (bytes)
  uint64 nproc;     // number of process
};
=======

struct sysinfo 
{
  uint64 freemem;   // 可用内存字节数
  uint64 nproc;     // 非 UNUSED 状态的进程数
};
>>>>>>> e540eec (Save my current syscall changes before switching to lab branch)
