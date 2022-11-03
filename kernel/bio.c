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

struct {
  struct spinlock buketslock[NBUKET];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head[NBUKET];
} bcache;

void
binit(void)
{
  struct buf *b;


  // 为每个hash桶初始化
  for(int i = 0; i < NBUKET; i++){
    initlock(&bcache.buketslock[i], "bcache.bucket");
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }
  
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    int i = b->blockno % NBUKET;
    b->next = bcache.head[i].next;
    b->prev = &bcache.head[i];
    initsleeplock(&b->lock, "buffer");
    bcache.head[i].next->prev = b;
    bcache.head[i].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int hash = blockno % NBUKET;
  acquire(&bcache.buketslock[hash]);

  // Is the block already cached?
  for(b = bcache.head[hash].next; b != &bcache.head[hash]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.buketslock[hash]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  // 先在自己的hash桶中寻找可替换的，若无再去偷其他桶
  for(b = bcache.head[hash].prev; b != &bcache.head[hash]; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.buketslock[hash]);
      acquiresleep(&b->lock);
      return b;
    }
  }


  for (int i = 0; i < NBUKET; i++)
  {
    if(i != hash){
      acquire(&bcache.buketslock[i]);
      for(b = bcache.head[i].prev; b != &bcache.head[i]; b = b->prev){
        if(b->refcnt == 0){
          b->dev = dev;
          b->blockno = blockno;
          b->valid = 0;    
          b->refcnt = 1;
          
          //将b从第i个桶脱出
          b->next->prev = b->prev;
          b->prev->next = b->next;
          
          //将b加入对应的hash桶
          b->next = bcache.head[hash].next;
          b->prev = &bcache.head[hash];
          bcache.head[hash].next->prev = b;
          bcache.head[hash].next = b;
          
          release(&bcache.buketslock[i]);
          release(&bcache.buketslock[hash]);
          acquiresleep(&b->lock);
          return b;
        }
      }
      release(&bcache.buketslock[i]);
    }
  }
  panic("bget: no buffers");
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

  int hash = b->blockno % NBUKET;
  acquire(&bcache.buketslock[hash]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[hash].next;
    b->prev = &bcache.head[hash];
    bcache.head[hash].next->prev = b;
    bcache.head[hash].next = b;
  }
  
  release(&bcache.buketslock[hash]);
}

void
bpin(struct buf *b) {
  int hash = b->blockno % NBUKET;
  acquire(&bcache.buketslock[hash]);
  b->refcnt++;
  release(&bcache.buketslock[hash]);
}

void
bunpin(struct buf *b) {
  int hash = b->blockno % NBUKET;
  acquire(&bcache.buketslock[hash]);
  b->refcnt--;
  release(&bcache.buketslock[hash]);
}


