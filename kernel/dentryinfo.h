/******************************************
 * 
 * DENTRYINFO.H
 * 
******************************************/

#ifndef _DENTRYINFO_H_
#define _DENTRYINFO_H_

#define DENTRYINFO_NAME_SIZE 32

struct dentryinfo {
    bool is_directory;
    unsigned int owner;
    long long size;
    char name[DENTRYINFO_NAME_SIZE];
    unsigned long i_inode;
    char folder_name[DENTRYINFO_NAME_SIZE];
    unsigned long folder_i_inode;

    int children_cnt;
    char children[128][DENTRYINFO_NAME_SIZE];
};

#endif
