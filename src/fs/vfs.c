#include "vfs.h"
#include <stddef.h>

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

VfsDirent_t *vread_dir(Vfs_t* node, uint32_t index) {
   if(node->read_dir && node->flag == VFS_DIRECTORY)
      return node->read_dir(node, index);
   return NULL;
}

Vfs_t* vfind_dir(Vfs_t* node, char* name) {
   if(node->find_dir && node->flag == VFS_DIRECTORY)
      return node->find_dir(node, name);
   return NULL;
}

Vfs_t vfs_node_new() {
   Vfs_t node;
   node.name[0] = '\0';
   node.flag = VFS_INVALID;

   node.read = NULL;
   node.write = NULL;

   node.open = NULL;
   node.read = NULL;

   node.read_dir = NULL;      
   node.find_dir = NULL;      
   return node;               
}                             
                              
int vfs_init() {              

}

