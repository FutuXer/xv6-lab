#include "kernel/types.h"
#include "user/user.h"
#include "stddef.h"

void
mapping(int n, int pd[])
{
  close(n);

  dup(pd[n]);

  close(pd[0]);
  close(pd[1]);
}

void
primes()
{

  int previous, next;
  
  int fd[2];

  if (read(0, &previous, sizeof(int)))
  {
 
    printf("prime %d\n", previous);
    // 创建管道
    pipe(fd);
    // 创建子进程
    if (fork() == 0)
    {
      // 子进程
      // 子进程将管道的写端口映射到描述符 1 上
      mapping(1, fd);
      // 循环读取管道中的数据
      while (read(0, &next, sizeof(int)))
      {
        // 如果该数不是管道中第一个数的倍数
        if (next % previous != 0)
        {
          // 写入管道
          write(1, &next, sizeof(int));
        }
      }
    }
    else
    {

      wait(NULL);

      mapping(0, fd);
      // 递归执行此过程
      primes();
    }  
  }  
}

int 
main(int argc, char *argv[])
{

  int fd[2];

  pipe(fd);

  if (fork() == 0)
  {
    mapping(1, fd);

    for (int i = 2; i < 36; i++)
    {

      write(1, &i, sizeof(int));
    }
  }
  else
  {
    wait(NULL);
    mapping(0, fd);
    primes();
  }
  exit(0);
}