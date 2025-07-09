#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int ticks; //tick数
  if (argc != 2) 
  {
    fprintf(2, "Usage: sleep <ticks>\n"); // fprintf(2, ...) 用于向标准错误输出打印
    exit(1); //失败
  }

  //将命令行参数（字符串）转换为整数
  //argv[1] 是用户传入的第一个参数（即滴答数）
  ticks = atoi(argv[1]); 

  sleep(ticks);

  exit(0);
}

