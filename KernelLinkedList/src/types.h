#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

struct list_head {
	struct list_head *next, *prev;
};

#endif