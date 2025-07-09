// user/trace.c
#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;
  char *nargv[MAXARG];

  if(argc < 3){
    fprintf(2, "Usage: %s mask command [args...]\n", argv[0]);
    exit(1);
  }

  // Set the trace mask
  // 设置跟踪掩码
  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "%s: trace failed\n", argv[0]);
    exit(1);
  }

  // Prepare arguments for the command to be executed
  // 为要执行的命令准备参数
  for(i = 2; i < argc; i++){
    nargv[i-2] = argv[i];
  }
  nargv[i-2] = 0;

  // Execute the command
  // 执行命令
  exec(nargv[0], nargv);
  
  fprintf(2, "%s: exec failed\n", nargv[0]);
  exit(1);
}
