#include "kprobe_manage.h"
#include "log_manage.h"
#include "procfs_manage.h"
#include "manage_pt_regs.h"

#include <linux/string.h>
#include <linux/socket.h>

static struct kprobe kps[MAX_PROBES];
static int count = 0;

static char *probe_names[] = {"sys_execve", "sys_connect"};


int init_kprobes(void)
{
    int i, probe_num, index;
    int ret = 0;
    for(i = 0; i < MAX_PROBES; ++i)
    {
        kps[i].symbol_name = NULL;
    }

    probe_num = 2;//sizeof(probe_names);

    if((probe_num > 0))
    {
        for(i = 0; i < probe_num; i++)
        {   
            init_kprobe(&kps[index], probe_names[i], handle_post);
            index++;
        }
    }
    
    for(i = 0; i < MAX_PROBES; i++)
    {
        if(kps[i].symbol_name != NULL)
        {
            ret = register_kprobe(&kps[i]);
            if(ret < 0)
            {
                printk("register failed, errno:%d\n", ret);
                goto end;
            }
            printk("resgistry:%s success!\n", kps[i].symbol_name);
        }
    }
   
    ret = 0;

end:
    if(ret < 0)
        destroy_kprobes();

    return ret;
}


void destroy_kprobes(void)
{
    int i;
    for(i = 0; i < MAX_PROBES; i++)
    {
        if(kps[i].symbol_name != NULL)
        {
            unregister_kprobe(&kps[i]);
        }
    }
}


int handle_pre(struct kprobe *p, struct pt_regs *regs)
{
    return 0;
}


void handle_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
    struct log_item *item;    

    if(is_monitor(current->pid))
    {
        printk("monitor process filter!\n");
        return;
    }

    item = get_log_item();
    
    item->pid = current->pid;
    item->ppid = current->parent->pid;
    strcpy(item->syscall, p->symbol_name);
    strcpy(item->name, current->comm);
    strcpy(item->pname, current->parent->comm);
    mange_regs(p->symbol_name, regs, item->buf, LOGSIZE);

}


int handle_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    return 0;
}


void init_kprobe(struct kprobe *p, char* name, posthandle handle)
{
    p->symbol_name = name;
    p->pre_handler = handle_pre;
    p->post_handler = handle;
    p->fault_handler = handle_fault;
}


void mange_regs(char *syscall, struct pt_regs *regs, char* buf, int len)
{
    int left = len;

    if (strcmp(syscall, "sys_execve") == 0)
    {
        char *path;
        char **argv;
        char **tmp;

        path = (char*)get_arg1(regs);
        argv = (char**)get_arg2(regs);

        strcat(buf, "objpath");
        strcat(buf, COLON);
        strcat(buf, path);
        strcat(buf, SPLIT);
        strcat(buf, "command");
        strcat(buf, COLON);

        tmp = argv;
        strcat(buf, path);
        while(*tmp != NULL)
        {
            if(strlen(buf) + strlen(*tmp) < LOGSIZE)
            strcat(buf, " ");
            strcat(buf, *tmp);
            tmp++;
        }
    }
    else if(strcmp(syscall, "sys_connect") == 0)
    {
        int fd = (int)get_arg1(regs);
        int len = get_arg3(regs);
        struct addrinfo * addr = (struct addrinfo *)buf;
        printk("fd:%d\n", fd);
        printk("len:%d\n", len);

        addr->len = len;
        addr->fd = fd;
        memcpy(addr->data, (char*)get_arg2(regs), len);
    }
}
