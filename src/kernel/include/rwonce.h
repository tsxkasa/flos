#ifndef _RWONCE_H
#define _RWONCE_H

#define WRITE_ONCE(x, val)                                                     \
  do {                                                                         \
    *(volatile typeof(x) *)&(x) = (val);                                       \
  } while (0)

#define READ_ONCE(x) (*(volatile typeof(x) *)&(x))

struct list_head {
  struct list_head *next, *prev;
};

#endif // _RWONCE_H
