/******************************************
 * 
 * DENTRYTEST.C
 * 
******************************************/
#include <stdbool.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/dentryinfo.h>
#include <linux/types.h>

#define CLR "\033[33;1m" // yellow
#define ECLR "\033[0m" // end of color string

#define __NR_dentryinfo 455

long dentryinfo_syscall(char* filename, struct dentryinfo* buffer, unsigned long len)
{
    return syscall(__NR_dentryinfo, filename, buffer, len);
}

void dentryinfo_print(struct dentryinfo *info) {
    printf("\n");

    printf(CLR"Name <inode>:"ECLR" %s <%lu>\n", info->name, info->i_inode);
    
    printf(CLR"Type:"ECLR" %s ", info->is_directory ? "Directory" : "File");
    printf(CLR"Size:"ECLR" %llu ", info->size);
    printf(CLR"Owner UID:"ECLR" %u\n", info->owner);

    printf(CLR"Parent directory <inode>:"ECLR" %s <%lu>\n", info->folder_name, info->folder_i_inode);

    if (info->is_directory) {
        printf(CLR"\nChildren:\n"ECLR);
        for(int i = 0; i < info->children_cnt; i++) {
            printf("%s ", info->children[i]);
        }
        printf("\n");
    }
    printf("\n");
};

int main(int argc, char *argv[])
{
    if(argc < 2) {
        printf("Usage:  dentrytest [filename | dirname]\n");
        return -1;
    }

    char* filename = argv[1];

    if (access(filename, F_OK) != 0) {
        printf("No such file or directory\n");
        return 1;
    }

    struct dentryinfo info;

    long rc;
    rc = dentryinfo_syscall(filename, &info, sizeof(info));

    if(rc < 0) {
        perror("System call is failed");
        return 1;
    }

    dentryinfo_print(&info);
    return 0;
}