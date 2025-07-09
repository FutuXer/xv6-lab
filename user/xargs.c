#include "kernel/types.h"   
#include "kernel/param.h"   
#include "user/user.h"      

// 定义行的最大长度，用于 buf 缓冲区
#define MAX_LINE_LEN 128 

int main(int argc, char *argv[]) {
    // 用于存储带 '/' 的完整命令路径
    char cmd_full_path[MAX_LINE_LEN]; 
    char buf[MAX_LINE_LEN]; // 存储从标准输入读取的单行内容
    char *params[MAXARG];   // 用于构建传递给 exec() 的命令及其参数列表
    char c;                 // 用于逐字符读取输入的缓冲区
    int i;                  // 循环变量
    int fixed_args_count;   // 存储固定参数（包括命令本身）的数量
    int line_buf_idx = 0;   // buf 中的当前写入位置
    int read_len;           // read() 返回的字节数

    // 检查参数个数是否合法
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command> [initial-args...]\n");
        exit(1);
    }

    // 1. 处理要执行的命令名，并构建其完整路径
    // 将第一个参数（命令名，如 "echo"）复制到 cmd_full_path，并在前面加上 '/'
    cmd_full_path[0] = '/';
    // strcpy 将字符串从 argv[1] 复制到 cmd_full_path 的第二个位置 (cmd_full_path + 1)
    // 确保 cmd_full_path 足够大以容纳 '/'+命令名+\0
    if (strlen(argv[1]) >= sizeof(cmd_full_path) - 1) {
        fprintf(2, "xargs: command name too long\n");
        exit(1);
    }
    strcpy(cmd_full_path + 1, argv[1]);
    // params[0] 指向这个带完整路径的字符串
    params[0] = cmd_full_path;

    // 2. 复制命令的其余固定参数
    // 注意这里从 i = 2 开始，因为 argv[1] (原始命令名) 已经处理了
    // params 的索引从 1 开始，对应 argv 的索引从 2 开始
    for (i = 2; i < argc; i++) {
        // 检查参数个数是否超过了限制 (params[0] 已经占用一个位置)
        if ((i - 1) >= MAXARG - 1) { 
            fprintf(2, "xargs: Too many initial arguments\n");
            exit(1);
        }
        params[i - 1] = argv[i];
    }
    // fixed_args_count 存储了固定参数（包括命令的完整路径）的数量
    // 也是后续从输入行读取的参数将要放置的起始索引
    fixed_args_count = i - 1;

    // 循环逐字符读取用户的输入
    while ((read_len = read(0, &c, 1)) == 1) {
        if (c == '\n') {
            // 遇到换行符，表示一行读取完毕
            buf[line_buf_idx] = '\0'; // 在行缓冲区的末尾添加空字符，使其成为有效的 C 字符串

            // 将整行内容（去除换行符后）作为新参数添加到 params
            // 检查参数个数是否超过了限制 (fixed_args_count + 1 是行参数 + NULL 的位置)
            if (fixed_args_count + 1 >= MAXARG) { 
                fprintf(2, "xargs: Too many arguments from input line\n");
                exit(1);
            }
            params[fixed_args_count] = buf; // 将整个行缓冲区作为单个参数
            
            // 终止参数列表：exec() 要求参数列表以 NULL (0) 结尾
            params[fixed_args_count + 1] = 0; 

            // 创建子进程来执行命令
            int pid = fork();
            if (pid < 0) {
                fprintf(2, "xargs: fork failed\n");
                exit(1);
            } else if (pid == 0) { // 子进程
                // 执行命令
                exec(params[0], params);
                // 如果 exec() 返回，说明执行失败
                fprintf(2, "xargs: exec failed for %s\n", params[0]);
                exit(1); // 子进程以失败状态退出
            } else { // 父进程
                // 父进程需要等待子进程结束
                wait(0); // wait(0) 等待任意子进程退出并返回其状态
            }

            // 重置行缓冲区索引，准备读取下一行
            line_buf_idx = 0;
        } else {
            // 尚未遇到换行符，将字符添加到行缓冲区
            if (line_buf_idx < sizeof(buf) - 1) { // 检查是否会超出缓冲区大小，预留一个空字符位置
                buf[line_buf_idx++] = c;
            } else {
                // 行太长，超出缓冲区限制。打印警告，但继续读取直到换行或 EOF。
                fprintf(2, "xargs: warning: line too long, truncating.\n");
                // 此时 buf 已经满了，不再写入，只消费输入流
            }
        }
    }

    // 处理最后一行（如果输入在没有换行符的情况下结束，且行缓冲区中有内容）
    // 只有当 read_len == 0 (EOF) 且 line_buf_idx > 0 时才处理
    if (read_len == 0 && line_buf_idx > 0) {
        buf[line_buf_idx] = '\0'; // 空字符终止最后一行
        
        // 将整行内容作为新参数添加到 params
        if (fixed_args_count + 1 >= MAXARG) {
            fprintf(2, "xargs: Too many arguments from input line\n");
            exit(1);
        }
        params[fixed_args_count] = buf;
        params[fixed_args_count + 1] = 0; // 空指针终止参数列表

        // 创建子进程来执行命令
        int pid = fork();
        if (pid < 0) {
            fprintf(2, "xargs: fork failed\n");
            exit(1);
        } else if (pid == 0) { // 子进程
            exec(params[0], params);
            fprintf(2, "xargs: exec failed for %s\n", params[0]);
            exit(1);
        } else { // 父进程
            wait(0);
        }
    }

    exit(0); // xargs 程序成功执行并退出
}
