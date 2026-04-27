#ifndef _KERNEL_PAGE_H
#define _KERNEL_PAGE_H

#define PAGE_SIZE 4096
#define KB_TO_PAGES(kb) (((kb) * 1024) / PAGE_SIZE)
#define BYTES_TO_PAGES(bytes) ((bytes) / PAGE_SIZE)

#define IS_PAGE_ALIGNED(num) (((num) & (PAGE_SIZE - 1)) == 0)

#endif // _KERNEL_PAGE_H
