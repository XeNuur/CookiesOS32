#pragma once
#include <stdint.h>
#include "vfs.h"

int devfs_check(Vfs_t* device);
int devfs_mount(Vfs_t* device);

int devfs_read_dir(Vfs_t*, uint32_t, VfsDirent_t*);
int devfs_find_dir(Vfs_t*, char*, Vfs_t**);

int mount_devfs_device(char* path);

