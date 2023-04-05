#include "vfs.h"
#include "fat.h"
#include "devfs.h"
#include "../kernel/kheap.h"
#include "../kernel/framebuffer.h"
#include "../kernel/device.h"
#include <stddef.h>
#include <string.h>

//TODO: charge this to hashmap
MountInfo_t *mount_points = 0;
uint32_t mount_count = 0;

uint32_t _find_relative_mount(char* file);

Vfs_t* fopen(char* path) {
   Vfs_t* file = malloc(sizeof(Vfs_t));
   uint32_t mnt_i = _find_relative_mount(path);
   MountInfo_t info = mount_points[mnt_i];

   if(strlen(path) <= strlen(info.path)) {
      free(file);
      return info.device->master;
   }
   char* real_path = path+strlen(info.path);
   int status = vfind_dir((info.device)->master, real_path, &file);

   if(!status) {
      free(file);
      return NULL;
   }
   vopen(file);
   return file;
}

int fclose(Vfs_t* node) {
   int stat = vclose(node);
   free(node);
   return stat;
}

int vread(Vfs_t* node, uint32_t offset, uint32_t size, char *ptr) {
   if(node->read)
      return node->read(node, offset, size, ptr);
   return -1;
}

int vwrite(Vfs_t* node, uint32_t offset, uint32_t size, char *ptr) {
   if(node->write)
      return node->write(node, offset, size, ptr);
   return -1;
}

int vopen(Vfs_t* node) {
   if(node->open)
      return node->open(node);
   return -1;
}

int vclose(Vfs_t* node) {
   if(node->close)
      return node->close(node);
   return -1;
}

int vread_dir(Vfs_t* node, uint32_t index, VfsDirent_t* dirent) {
   if(node->read_dir && (node->flag == VFS_DIRECTORY || node->flag == VFS_FILESYSTEM))
      return node->read_dir(node, index, dirent);
   return -1;
}

int vfind_dir(Vfs_t* node, char* name, Vfs_t** target) {
   if(node->read_dir && (node->flag == VFS_DIRECTORY || node->flag == VFS_FILESYSTEM))
      return node->find_dir(node, name, target);
   return -1;
}

Vfs_t vfs_node_new() {
   Vfs_t node;
   node.name[0] = '\0';
   node.flag = VFS_INVALID;

   node.read = NULL;
   node.write = NULL;

   node.open = NULL;
   node.close = NULL;
   node.read = NULL;

   node.read_dir = NULL;      
   node.find_dir = NULL;      

   node.master = NULL;
   return node;               
}                             

//TODO: Major refractor over this code
int vmount_device(Vfs_t* device, char* path, uint32_t type) {
   char* err_msg = "Undefined";
   MountInfo_t info;
   if(type == 0)  {
      if(!fat_check(device)) {
         err_msg = "Not a Fat16 device";
         goto exit_with_err;
      }
   
      if(!fat_mount(device)){
         err_msg = "fat_mount(..) returned with error";
         goto exit_with_err;
      }
      info.path = path;
      info.device = device;
   }
   else if(type == 1) {
      if(!devfs_check(device)) {
         err_msg = "Not a devfs device";
         goto exit_with_err;
      }
      if(!devfs_mount(device)){
         err_msg = "dev_mount(..) returned with error";
         goto exit_with_err;
      }
      info.path = path;
      info.device = device;
   } 

   mount_points[mount_count++] = info;
   term_printf("Mounted %s in path: %s\n", device->name, path);
   return 1;
exit_with_err:
   term_printf("Unable to mount device (%s)\n", err_msg);
   return 0;
}

int _vfs_mount_init() {
   mount_points = malloc(sizeof(MountInfo_t));
   mount_count = 0;
}
                              
int vfs_init() {              
   _vfs_mount_init();
}

uint32_t _find_relative_mount(char* _path) {
   char* mount_path = malloc(strlen(_path)+1);
   memcpy(mount_path, _path, strlen(_path)+1);

   for(;;) {
      for(uint32_t i=0; i<mount_count; ++i) {
         if(strcmp(mount_path, mount_points[i].path) == 0) {
            free(mount_path);
            return i;
         }
      }
      if(strcmp(mount_path, "/") == 0) {
         break;
      }
      strdeltoc(mount_path, '/');
   }
   free(mount_path);
   return 0;
}

