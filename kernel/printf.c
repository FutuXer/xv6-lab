//
// formatted console output -- printf, panic.
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

volatile int panicked = 0;

// lock to avoid interleaving concurrent printf's.
static struct {
  struct spinlock lock;
  int locking;
} pr;

static char digits[] = "0123456789abcdef";

static void
printint(int xx, int base, int sign)
{
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  consputc('0');
  consputc('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    consputc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the console. only understands %d, %x, %p, %s.
void
printf(char *fmt, ...)
{
  va_list ap;
  int i, c, locking;
  char *s;

  locking = pr.locking;
  if(locking)
    acquire(&pr.lock);

  if (fmt == 0)
    panic("null fmt");

  va_start(ap, fmt);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printint(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      printptr(va_arg(ap, uint64));
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&pr.lock);
}

void
panic(char *s)
{
  pr.locking = 0;
  backtrace();
  printf("panic: ");
  printf(s);
  printf("\n");
  panicked = 1; // freeze uart output from other CPUs
  for(;;)
    ;
}

void
printfinit(void)
{
  initlock(&pr.lock, "pr");
  pr.locking = 1;
}

// backtrace() 函数实现
void
backtrace(void)
{
  uint64 fp = r_fp(); // 获取当前帧指针
  struct proc *p = myproc(); // 获取当前进程

  // 获取当前进程的内核堆栈的底部和顶部地址。
  // xv6 为每个进程分配一个页大小的内核堆栈，从 p->kstack 开始。
  uint64 stack_bottom = p->kstack;
  uint64 stack_top = p->kstack + PGSIZE; // 栈的最高有效地址 + 1

  printf("backtrace:\n");

  // 遍历堆栈帧
  // 循环条件：
  // 1. fp 必须非零。
  // 2. fp 必须在当前进程的内核堆栈范围内 (stack_bottom < fp < stack_top)。
  //    注意：fp 应该总是在 stack_bottom 和 stack_top 之间，且地址是增加的（指向栈帧的高地址部分）。
  //    由于栈从高地址向低地址增长，所以 fp 通常在栈的较高部分，并指向前一个栈帧的 fp。
  //    fp 本身不能是 kstack 的起始地址（最低地址），所以用 `>`。
  while (fp != 0 && fp > stack_bottom && fp < stack_top) {
    // 返回地址（Return Address, RA）通常在当前帧指针的固定偏移量 -8 处
    // fp 寄存器 s0 存储的值是上一个栈帧的 fp。
    // 当前函数的返回地址（RA）存储在当前 fp 所在位置的 -8 偏移处。
    uint64 ra = *(uint64*)(fp - 8);
    printf("0x%p\n", ra);

    // 移动到上一个堆栈帧的帧指针
    // 上一个堆栈帧的帧指针通常在当前帧指针的固定偏移量 -16 处
    fp = *(uint64*)(fp - 16);
  }
}