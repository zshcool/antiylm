#ifndef LOG_ITEM_H_
#define LOG_ITEM_H_

#include <linux/slab.h>

#define LOGSIZE 1024
#define NAMESIZE 16
#define COLON "&:"
#define SPLIT "&/"

struct log_item
{
    int pid;
    int ppid;
    char syscall[NAMESIZE];
    char name[NAMESIZE];
    char pname[NAMESIZE];
    char buf[LOGSIZE];
};

struct addrinfo
{
    int len;
    int fd;
    char data[256];
};	

struct log_item* init_log_item(void);
void release_log_item(struct log_item* log_item);


#endif
