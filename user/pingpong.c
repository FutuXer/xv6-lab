#include "kernel/types.h" 
#include "user/user.h"    
int main(int argc, char *argv[])
{
  int p2c_pipe[2]; //父进程到子进程
  int c2p_pipe[2]; //子进程到父进程
  char byte_buf[1]; //缓冲区
  int pid;       

  if (pipe(p2c_pipe) < 0) 
  {
    fprintf(2, "pingpong: failed to create parent-to-child pipe\n");
    exit(1);
  }

  if (pipe(c2p_pipe) < 0) 
  {
    fprintf(2, "pingpong: failed to create child-to-parent pipe\n");
    exit(1);
  }

  //创建子进程
  pid = fork();

  if (pid < 0)
  {
    //fork 失败
    fprintf(2, "pingpong: fork failed\n");
    exit(1);
  } 
  else if (pid == 0) 
  {
    //子进程
    close(p2c_pipe[1]);//只读
    close(c2p_pipe[0]);//只写

    //从父进程读取一个字节
    if (read(p2c_pipe[0], byte_buf, 1) != 1) 
    {
      fprintf(2, "pingpong: child failed to read from pipe\n");
      exit(1);
    }

    fprintf(1, "%d: received ping\n", getpid()); // fprintf(1, ...) 用于向标准输出打印

    //父进程写入一个字节
    if (write(c2p_pipe[1], byte_buf, 1) != 1) 
    {
      fprintf(2, "pingpong: child failed to write to pipe\n");
      exit(1);
    }

    //关闭
    close(p2c_pipe[0]);
    close(c2p_pipe[1]);

    exit(0); 
  } 
  else 
  {
    //父进程只写
    close(p2c_pipe[0]);
    //父进程只读
    close(c2p_pipe[1]);

    if (write(p2c_pipe[1], byte_buf, 1) != 1) 
    {
      fprintf(2, "pingpong: parent failed to write to pipe\n");
      exit(1);
    }

    //避免僵尸进程
    wait(0); 

    //从子进程读取
    if (read(c2p_pipe[0], byte_buf, 1) != 1) 
    {
      fprintf(2, "pingpong: parent failed to read from pipe\n");
      exit(1);
    }

    //打印
    fprintf(1, "%d: received pong\n", getpid());

    close(p2c_pipe[1]);
    close(c2p_pipe[0]);

    exit(0); 
  }
}

