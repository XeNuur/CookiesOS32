#pragma once
#include <stddef.h>
#include <stdint.h>
#include "../fs/vfs.h"

typedef struct vfs_t device_t;

void devices_init();
uint32_t devices_add(device_t);
device_t* devices_at(uint32_t);
uint32_t devices_number();
