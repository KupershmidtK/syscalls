## Инструменты для сборки.
Выполняется на виртуальной машине с Ubuntu.

Перед началом работ устанавливаем необходимые инструменты.

> sudo apt update && sudo apt upgrade -y \
> sudo apt install build-essential libncurses-dev libssl-dev libelf-dev bison flex -y

Создаем в домашней директории каталог *kernel* и заходим в него.
> mkdir kernel \
> cd kernel

Скачиваем и распаковываем последний дистрибутив ядра Linux.
> wget -P https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.6.3.tar.xz \
> tar -xvf linux-6.6.3.tar.xz

Заходим в созданную папку и создаем папку для новых syscall вызовов
> cd linux-6.6.3 \
> mkdir mysyscalls

Добавляем нашу папку в список папок дистрибутива Linux. 
Открываем в редакторе файл Kbuild и добавляем строку
> obj-y += mysyscalls/

## syscall для memblock
Заходим в папку *mysyscalls* и создаем файлы *memblockinfo.c, memblockinfo.h, Makefile*. 
```
/***********************************************
*  
* MEMBLOCKINFO.H
*
************************************************/

#ifndef _MEMBLOCKDATA_H_
#define _MEMBLOCKDATA_H_

enum memblockinfo_flags {
	MEMBLOCKINFO_NONE		= 0x0,	/* No special request */
	MEMBLOCKINFO_HOTPLUG	= 0x1,	/* hotpluggable region */
	MEMBLOCKINFO_MIRROR		= 0x2,	/* mirrored region */
	MEMBLOCKINFO_NOMAP		= 0x4,	/* don't add to kernel direct mapping */
	MEMBLOCKINFO_DRIVER_MANAGED = 0x8	/* always detected via a driver */
};

struct memblockinfo_region {
	long long unsigned base;
	long long unsigned size;
	enum memblockinfo_flags flags;
};

struct memblockinfo_type {
	unsigned long cnt;
	unsigned long max;
	long long unsigned total_size;
	struct memblockinfo_region regions[128];
	char name[32];
};

struct memblockinfo {
    bool bottom_up;  /* is bottom up direction? */
	long long unsigned current_limit;
	struct memblockinfo_type memory;
	struct memblockinfo_type reserved;
};

#endif
```
```
/***********************************************
*  
* MEMBLOCKINFO.C
*
************************************************/

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <linux/memblock.h>
#include "memblockinfo.h"

static struct memblockinfo memblockinfo;

SYSCALL_DEFINE2(memblockinfo, void*, buffer, unsigned long, len)
{
    if(len < sizeof(struct memblockinfo))
        return -EINVAL;

        
    // ---------- copy memblock ---------------
    memblockinfo.bottom_up = memblock.bottom_up;
    memblockinfo.current_limit = memblock.current_limit;

    memblockinfo.memory.cnt = memblock.memory.cnt;
    memblockinfo.memory.max = memblock.memory.max;
    memblockinfo.memory.total_size = memblock.memory.total_size;
    strcpy(memblockinfo.memory.name, memblock.memory.name);

    memblockinfo.reserved.cnt = memblock.reserved.cnt;
    memblockinfo.reserved.max = memblock.reserved.max;
    memblockinfo.reserved.total_size = memblock.reserved.total_size;
    strcpy(memblockinfo.reserved.name, memblock.reserved.name);

    struct memblock_region* region = NULL;
    int i = 0;
    for_each_mem_region(region) {
        memblockinfo.memory.regions[i].flags = (enum memblockinfo_flags)region->flags;
        memblockinfo.memory.regions[i].base = region->base;
        memblockinfo.memory.regions[i].size = region->size;
        i++;
    }

    i = 0;
    for_each_reserved_mem_region(region) {
        memblockinfo.reserved.regions[i].flags = (enum memblockinfo_flags)region->flags;
        memblockinfo.reserved.regions[i].base = region->base;
        memblockinfo.reserved.regions[i].size = region->size;
        i++;
    }

    if (copy_to_user(buffer, &memblockinfo, sizeof(memblockinfo)))
        return -EFAULT;
    
    return 0;
}
```
```
# MAKEFILE

obj-y := memblockinfo.o
```
## Регистрация syscall
Переходим в папку с дистрибутивом Linux *~/kernel/linux-6.6.3* и
открываем на редактирование файл *./include/linux/syscalls.h*. В конце файла добавляем строку:
> asmlinkage long sys_memblockinfo(void* buffer, unsigned long len);

Открываем на редактирование файл *./arch/x86/entry/syscalls/syscall_64.tbl*.
Находим последнюю регистрацию syscall и добавляем свою запись со следующим номером 454.

![img_1.png](img_1.png)

## Конфигурация и сборка ядра Linux
Переходим в папку с дистрибутивом Linux *~/kernel/linux-6.6.3*

Копируем текущую конфигурацию Linux
>cp -v /boot/config-$(uname -r) .config

Так как структура **memblock** уничтожается после начальной стадии загрузки Linux, 
для ее сохранения необходимо включить флаг CONFIG_ARCH_KEEP_MEMBLOCK. 

Для этого редактируем файл с флагами *./mm/Kconfig*.
Открываем файл на редактирование, находим запись

```
config ARCH_KEEP_MEMBLOCK
    bool
```
и меняем на
```
config ARCH_KEEP_MEMBLOCK
   bool 'Keep Memblock'
```
В папке *linux-6.6.3* запускаем команду
> make menuconfig

Соглашаемся с генерацией новых записей
и после появления графического окна переходим в пункт *Memory Management options*, устанвливаем флаг в пункте *Keep Memblock*,
сохраняемся и выходим.

![img.png](img.png)

Чтобы подавить ошибки сборки ядра на Ubuntu дополнительно выполняем следующие команды
> scripts/config --disable SYSTEM_TRUSTED_KEYS \
> scripts/config --disable SYSTEM_REVOCATION_KEYS

После завершения конфигурирования запускаем сборку ядра.
> make -j$(nproc)

После завершения сборки устанавливаем модули и ядро Linux, после чего перезагружаем виртуальную машину.
> sudo make modules_install \
> sudo make install \
> sudo reboot now

После перезагрузки проверяем версию Linux
> uname -mrs

На экране должна появиться информация *Linux 6.6.3 x86_64*

## Вызов syscall из приложений
Копируем новые заголовочные файлы в общедоступный каталог
> sudo cp ~/kernel/linux-6.6.3/mysyscall/memblockinfo.h /usr/include/linux
> 
В домашнем каталоге создаем папку sc_test. В этой папке создаем файл *memblocktest.c*
```
/*****************************************************
*
* MEMBLOCKTEST.C
*
*****************************************************/

#include <stdbool.h>
#include <linux/kernel.h>
#include <linux/memblockinfo.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define __NR_memblockinfo 454

long memblockinfo_syscall(struct memblockinfo* buffer, unsigned long len)
{
    return syscall(__NR_memblockinfo, buffer, len);
}

static void print_memblockinfo_type(struct memblockinfo_type* memtype) {
    printf("\n");
    printf("Name: %s\n", memtype->name);
    printf("Number of regions: %ld\n", memtype->cnt);
    printf("Size of the allocated array: %ld\n", memtype->max);
    printf("Size of all regions: %llu\n", memtype->total_size);


    printf("base\t\tsize\t\tflag\n");
    printf("----------------------------------------------------\n");
    
    struct memblockinfo_region* region = NULL;
    for (region = memtype->regions;
	     region < (memtype->regions +memtype->cnt);
	     region++) {

        char* flags = 
            region->flags == MEMBLOCKINFO_NONE
            ? "MEMBLOCK_NONE" :
            region->flags == MEMBLOCKINFO_HOTPLUG
            ? "MEMBLOCK_HOTPLUG" :
            region->flags == MEMBLOCKINFO_MIRROR
            ? "MEMBLOCK_MIRROR" :
            region->flags == MEMBLOCKINFO_NOMAP
            ? "MEMBLOCK_NOMAP" :
            region->flags == MEMBLOCKINFO_DRIVER_MANAGED
            ? "MEMBLOCK_DRIVER_MANAGED" :
            "NO FLAGS";

        printf(
            "%12llu\t%12llu\t%s\n",
            region->base,
            region->size,
            flags);
    }
} 

void print_memblockinfo(struct memblockinfo* memblock) {
    printf(">>> Struct MEMBLOCK information <<<\n\n");

    printf("Bottom Up: %s\n", memblock->bottom_up ? "TRUE" : "FALSE");
    printf("Current limit: %llu\n", memblock->current_limit);

    print_memblockinfo_type(&memblock->memory);
    print_memblockinfo_type(&memblock->reserved);
}

int main(int argc, char *argv[])
{
    struct memblockinfo memblock;

    long rc;
    rc = memblockinfo_syscall(&memblock, sizeof(memblock));

    if(rc < 0) {
        perror("System call is failed");
        return 1;
    }

    print_memblockinfo(&memblock);
    return 0;
}
```
Компилируем файл и запускаем на исполнение
> gcc -o memblocktest memblocktest.c \
> ./memblocktest

На экран выводится информация о структуре memblock

![img_2.png](img_2.png)