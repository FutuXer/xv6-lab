#include "kernel/types.h" 
#include "user/user.h"    
//从左侧邻居读取读端描述符
void sieve(int left_pipe_read_fd)
{
  int prime;          //当前找到的素数
  int n;              //左侧读取的数字
  int right_pipe[2];  //新创建的管道
  int pid;            //返回的进程 ID

  //当前素数
  if (read(left_pipe_read_fd, &prime, sizeof(prime)) == 0) 
  {
    close(left_pipe_read_fd);
    exit(0);
    return;
  }

  //打印当前找到的素数
  fprintf(1, "prime %d\n", prime);

  //为下一个进程创建pipe
  if (pipe(right_pipe) < 0) 
  {
    fprintf(2, "primes: failed to create right pipe\n");
    close(left_pipe_read_fd);
    exit(1);
    return;
  }

  //创建子进程
  pid = fork();

  if (pid < 0) 
  {
    //失败
    fprintf(2, "primes: fork failed\n");
    close(left_pipe_read_fd);
    close(right_pipe[0]);
    close(right_pipe[1]);
    exit(1);
    return;
  } 
  else if (pid == 0) 
  {
    close(right_pipe[1]);
    close(left_pipe_read_fd);

    //递归调用sieve
    sieve(right_pipe[0]);

    exit(0); 
    return;
  } 
  else 
  {
    //父进程
    //关闭右侧管道的读端 
    close(right_pipe[0]);

    //继续读取
    while (read(left_pipe_read_fd, &n, sizeof(n)) > 0) 
    {
      //如果数字 n 不是prime的倍数，则将其写入右侧管道
      if (n % prime != 0) 
      {
        if (write(right_pipe[1], &n, sizeof(n)) != sizeof(n)) 
        {
          fprintf(2, "primes: parent failed to write to right pipe\n");
          close(left_pipe_read_fd);
          close(right_pipe[1]);
          exit(1);
          return;
        }
      }
    }

    close(left_pipe_read_fd);
    close(right_pipe[1]); 

    wait(0);

    exit(0);
    return;
  }
}

int main(int argc, char *argv[])
{
  int p[2]; //初始管道
  int i;    //循环变量
  int pid;  //回的进程 ID

  if (pipe(p) < 0) 
  {
    fprintf(2, "primes: failed to create initial pipe\n");
    exit(1);
    return 1;
  }

  pid = fork();

  if (pid < 0) 
  {
    fprintf(2, "primes: fork failed\n");
    close(p[0]);
    close(p[1]);
    exit(1);
    return 1;
  } 
  else if (pid == 0) 
  {
    close(p[1]);

    sieve(p[0]);

    exit(0);
    return 0;
  } 
  else 
  {
    close(p[0]);

    //写入管道
    for (i = 2; i <= 35; i++) {
      if (write(p[1], &i, sizeof(i)) != sizeof(i)) 
      {
        fprintf(2, "primes: parent failed to write initial numbers\n");
        close(p[1]);
        exit(1);
        return 1;
      }
    }

    close(p[1]);

    wait(0);

    exit(0);
    return 0;
  }
}

