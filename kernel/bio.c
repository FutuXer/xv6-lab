// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUCKETCNT 13

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} bcache;

struct bucket{
  struct spinlock lock;
  struct buf head;
}hashtable[BUCKETCNT]; // 13

void
binit(void)
{
  struct buf *b;
  int i;

  initlock(&bcache.lock, "bcache");

  // // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;

  for(i = 0; i < BUCKETCNT; i++){
    initlock(&hashtable[i].lock, "bcache_hash");
    hashtable[i].head.prev = &hashtable[i].head;
    hashtable[i].head.next = &hashtable[i].head;
  }

  for(i = 0, b = bcache.buf; b < bcache.buf+NBUF; b++, i = (i + 1) % BUCKETCNT){
    b->next = hashtable[i].head.next;
    b->prev = &hashtable[i].head;
    initsleeplock(&b->lock, "buffer");
    hashtable[i].head.next->prev = b;
    hashtable[i].head.next = b;

    b->time_stamp = 0;
    b->now_hash = i;
  }

}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  struct buf *lru=0;
  uint hash = blockno % BUCKETCNT;
  uint min_time_stamp = ticks + 114514;

  acquire(&hashtable[hash].lock);

  for(b = hashtable[hash].head.next; b != &hashtable[hash].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&hashtable[hash].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // printf("ticks = %d\n", ticks);
  // for(b = bcache.buf; b < bcache.buf+NBUF; b++){
  //   if(b->refcnt == 0 && b->time_stamp <= min_time_stamp) {
  //     lru = b;
  //     min_time_stamp = b->time_stamp;
  //   }
  // }

  for(int i = (hash + 1) % BUCKETCNT; i != hash; i = (i + 1) % BUCKETCNT){
    acquire(&hashtable[i].lock);

    for(b = hashtable[i].head.next; b != &hashtable[i].head; b = b->next){
      if(b->refcnt == 0 && b->time_stamp < min_time_stamp) {
        lru = b;
        min_time_stamp = b->time_stamp;
      }
    }

    if(!lru) {
      release(&hashtable[i].lock);
      continue;
    }

    b = lru;
    b->next->prev = b->prev;
    b->prev->next = b->next;
    release(&hashtable[i].lock);


    b->dev = dev;
    b->blockno = blockno;
    b->valid = 0;
    b->refcnt = 1;
    b->now_hash = hash;


    b->next = hashtable[hash].head.next;
    b->prev = &hashtable[hash].head;

    acquiresleep(&b->lock);

    hashtable[hash].head.next->prev = b;
    hashtable[hash].head.next = b;
    release(&hashtable[hash].lock);

    return b;
  }
  // printf("min_time_stamp = %d\n", min_time_stamp);

  if(!lru)
    panic("bget: no buffers");
  return lru;
}
// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&hashtable[b->now_hash].lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->time_stamp = ticks;
  }
  release(&hashtable[b->now_hash].lock);
}

void
bpin(struct buf *b) {
  // printf("!!!\n");
  acquire(&hashtable[b->now_hash].lock);
  b->refcnt++;
  release(&hashtable[b->now_hash].lock);
}

void
bunpin(struct buf *b) {
  acquire(&hashtable[b->now_hash].lock);
  b->refcnt--;
  release(&hashtable[b->now_hash].lock);
}


