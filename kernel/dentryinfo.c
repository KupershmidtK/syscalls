/***************************
* 
*   DENTRYINFO.C
*
*****************************/

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <linux/path.h>
#include <linux/dcache.h>
#include <linux/namei.h>
#include "dentryinfo.h"

static struct dentryinfo dentryinfo;

SYSCALL_DEFINE3(dentryinfo, const char __user *, filename, void __user *, buffer, unsigned long, len) {
    if(len < sizeof(struct dentryinfo))
        return -EINVAL;

    struct file * fi;
    struct dentry * thedentry;
    struct dentry * parentdentry;
    struct dentry * curdentry;
    const char * curname = NULL;

    struct filename *fname = getname(filename);
    if(!fname) {
        pr_err("Can't read path file\n");
        return -EINVAL;           
    }

    fi = filp_open(fname->name, O_RDONLY, 0);
    if(!fi) {
        pr_err("Can't open file: %s\n", fname->name);
        return -EINVAL;          
    }

    thedentry = fi->f_path.dentry;
    parentdentry = thedentry->d_parent;

    dentryinfo.is_directory = S_ISDIR(thedentry->d_inode->i_mode);
    strncpy(dentryinfo.name, thedentry->d_name.name, DENTRYINFO_NAME_SIZE);
    dentryinfo.i_inode = thedentry->d_inode->i_ino;
    dentryinfo.owner = thedentry->d_inode->i_uid.val;
    dentryinfo.size = thedentry->d_inode->i_size;
 
    strncpy(dentryinfo.folder_name, parentdentry->d_name.name, DENTRYINFO_NAME_SIZE);
    dentryinfo.folder_i_inode = parentdentry->d_inode->i_ino;

    dentryinfo.children_cnt = 0;
    list_for_each_entry(curdentry, &thedentry->d_subdirs, d_child) {
        struct inode* inode = d_inode(curdentry);
        if ( !inode || !S_ISDIR(inode->i_mode)) 
            continue;

        curname = curdentry->d_name.name;
        pr_info("child: %s", curname);
        strncpy(dentryinfo.children[dentryinfo.children_cnt], curname, DENTRYINFO_NAME_SIZE);   
        
        if(dentryinfo.children_cnt == 127) 
            break;
        dentryinfo.children_cnt++;
    }

    filp_close(fi, NULL);
    if (copy_to_user(buffer, &dentryinfo, len))
        return -EFAULT;

    return 0;
}