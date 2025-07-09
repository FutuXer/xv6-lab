#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/sysinfo.h" // 包含 sysinfo 结构体的定义

void
sysinfotest()
{
  struct sysinfo si;

  printf("sysinfotest: start\n");
  sysinfo(&si);
  printf("sysinfo: freemem=%d nproc=%d\n", si.freemem, si.nproc);
  if(si.freemem > 0 && si.nproc > 0)
    printf("sysinfotest: OK\n");
  else
    printf("sysinfotest: FAIL\n");
}

int
main()
{
  sysinfotest();
  exit(0);
}