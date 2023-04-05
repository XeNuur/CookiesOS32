#include "devfs.h"
#include "../kernel/device.h"
#include "../kernel/kheap.h"
#include <string.h>

int devfs_check(Vfs_t* device) {
   if(device->flag != VFS_BLOCK_DEV) {
      return 0;
   }
   return 1;
}

int devfs_mount(Vfs_t* device) {
   if(device->flag != VFS_BLOCK_DEV) {
      return 0;
   }
   Vfs_t *node = malloc(sizeof(Vfs_t));
   memcpy(node->name, "DEVFS", strlen("DEVFS"));
   node->flag = VFS_FILESYSTEM;
   node->read_dir = devfs_read_dir;
   node->find_dir = devfs_find_dir;
   node->master = device;

   device->master = node;
   return 1;
}

int devfs_read_dir(Vfs_t* dev, uint32_t index, VfsDirent_t* dirent) {
   if(index > devices_number())
      return 0;
   char* tar_name = devices_at(index)->name;
   memcpy(dirent->name, tar_name, strlen(tar_name));
   return 1;
}

int devfs_find_dir(Vfs_t* dev, char* path, Vfs_t** target) {
   int status = 0;
   for (int i=0; i<devices_number(); i++) {
      Vfs_t* dev = devices_at(i);
      if(strcmp(dev->name, path) == 0) {
         *target = dev;  
         status = 1;
         break;
      }
   }
   return status;
}

int mount_devfs_device(char* path) {
   Vfs_t device_handler = vfs_node_new();

   const char* name = "devman";
   memcpy(device_handler.name, name, strlen(name)+1);
   device_handler.flag = VFS_BLOCK_DEV;
   uint32_t dev_id = devices_add(device_handler);

   vmount_device(devices_at(dev_id), path, 1);
}

