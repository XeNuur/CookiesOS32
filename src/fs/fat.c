#include "fat.h"
#include "../kernel/device.h"
#include "../kernel/panic.h"
#include "../kernel/framebuffer.h"
#include "../kernel/kheap.h"
#include "vfs.h"

#include <string.h>

#define FAT_MAX_SUBNAME (16)

int _parse_bpb(Vfs_t *dev, FatBpB_t *bpb) {
   char* buffer = malloc(512);
   vread(dev, 0, 1, buffer);

   memcpy(bpb, buffer+3, sizeof(FatBpB_t));
   free(buffer);
   return 0;
}

int fat_check(Vfs_t* device) {
   if(device->flag != VFS_BLOCK_DEV || !device->read) {
      return 1;
   }
   char* buffer = malloc(512);
   vread(device, 0, 1, buffer);
   uint8_t sig[] = {
      0xEB, 0x3C, 0x90
   };

   int status = 1;
   for(int i=0; i<3; ++i)
      if(sig[i] != buffer[i]) {
         status = 0;
         break;
      }
   free(buffer);
   return 0;
}

int fat_mount(Vfs_t* device) {
   if(device->flag != VFS_BLOCK_DEV || !device->read)
      return 0;
   Vfs_t *node = malloc(sizeof(Vfs_t));
   memcpy(node->name, "FAT16", strlen("FAT16"));
   node->flag = VFS_FILESYSTEM;
   node->read_dir = fat_read_dir;
   node->find_dir = fat_find_dir;
   node->master = device;

   device->master = node;
   return 0;
}

int fat_file_create(Vfs_t* node, char* path, Vfs_t* device) {
   *node = vfs_node_new();
   memcpy(node->name, path, VFS_NAME_MAX);

   node->read = fat_file_read;
   node->master = device;
   node->flag = VFS_FILE;
}

int fat_dir_create(Vfs_t* node, char* path, Vfs_t* device) {
   *node = vfs_node_new();
   memcpy(node->name, path, VFS_NAME_MAX);

   node->read_dir = fat_dir_readdir;
   node->find_dir = fat_dir_finddir;
   node->flag = VFS_DIRECTORY;
}

int _fat_node_create(Vfs_t* node, char* path, Vfs_t* device, FatEntry_t* entry) {
   if(entry->attributes == 0x10)
      return fat_dir_create(node, path, device);
   return fat_file_create(node, path, device);
}

FatEntry_t *_fat_get_filehandle_buffer(char* path, char* b512) {
   int i=0;
   while(i < 512) {
      FatEntry_t *entry = (void*)(b512 + i);
      if(memcmp(entry->filename, path, strlen(path)) == 0)
         return entry;
      i += 32;
   }
   return (FatEntry_t*)NULL;
}

FatEntry_t *_fat_get_filehandle_rootdir(Vfs_t* dev, char* path) {
   FatBpB_t fatbpb;
   _parse_bpb(dev, &fatbpb);
   uint8_t* buffer = malloc(512);

   uint32_t root_lba = fatbpb.NumberOfFats * fatbpb.SectorsPerFat + fatbpb.ReservedSectors;
   vread(dev, root_lba, 1, buffer);
   FatEntry_t* result_ptr = _fat_get_filehandle_buffer(path, buffer);

   FatEntry_t* status = malloc(sizeof(FatEntry_t));
   memcpy(status, result_ptr, sizeof(FatEntry_t));

   free(buffer);
   return status;
}

int fat_file_read_with_handle(Vfs_t* file, FatEntry_t handle, uint32_t size, char* ptr) {
   if(!(file->flag == VFS_FILE || file->flag == VFS_DIRECTORY))
      return 1;

   FatBpB_t fatbpb;
   _parse_bpb(file->master, &fatbpb);
   uint32_t root_lba = fatbpb.NumberOfFats * fatbpb.SectorsPerFat + fatbpb.ReservedSectors;

   uint32_t buffer_size = 512 * fatbpb.SectorsPerCluster;
   uint8_t* buffer = malloc(buffer_size);

   uint16_t current_cluster = handle.starting_cluster;
   uint32_t index = 0;
   while(current_cluster <= FAT16_EOF) {
      uint32_t data_sec = (root_lba+32);
      uint32_t lba_ctx = data_sec + (current_cluster-2) * fatbpb.SectorsPerCluster;
      
      vread(file->master, lba_ctx, fatbpb.SectorsPerCluster, buffer);

      for(uint32_t i=0; i<buffer_size; i++) {
         ptr[index] = ((char*)buffer)[i];
         index++;
         if(index > size)
            break;
      }
      uint32_t frist_fat = fatbpb.ReservedSectors;

      uint32_t fat_offset = current_cluster * 2;
      uint32_t fat_sector = frist_fat + (fat_offset / fatbpb.BytesPerSector);
      uint32_t ent_offset = fat_offset % fatbpb.BytesPerSector;

      vread(file->master, fat_sector, 1, buffer);
      current_cluster = *(uint16_t*)&buffer[ent_offset];
   }
   free(buffer);
   return 0;
}

char* strtok(char* path, char ch) {
   static char* input = NULL;
   
   if(path != NULL)
      input = path;
   if(input == NULL) 
      return NULL;

   char* result = malloc(strlen(input)+1);
   uint32_t i=0;

   for(; input[i] != '\0'; i++) {
      if(input[i] != ch) {
         result[i] = input[i];
         continue;
      }
      result[i] = '\0';
      input = input + i + 1;

      if(i == 0)
         return strtok(NULL, ch);
      return result;
   }
   result[i] = '\0';
   input = NULL;
   return result;
}

int _fat_basename(char* path, char** target) {
   size_t i = strlen(path);
   i--;
   while(i) {
      i--;
      if(path[i] != '/')
         continue;
      i++; 
      break;
   }
   *target = path+i;
   return i;
}

int _fat_get_filehandle(Vfs_t* file, FatEntry_t* retent) {
   char *nodename = strtok(file->name, '/');
   char* buffer = malloc(512);

   FatEntry_t* result = _fat_get_filehandle_rootdir(file->master, nodename);
   if(!result) 
      return 1;
   FatEntry_t *entry = result;
   int status = 1;
   
   if(result->attributes != 0x10) {
      free(nodename);
      goto success_break;
   }
   fat_file_read_with_handle(file, *result, 512, buffer);

   while(nodename != NULL) {
      free(nodename);
      nodename = strtok(0, '/');

      entry = _fat_get_filehandle_buffer(nodename, buffer);
      if(!entry) {
         free(nodename);
         goto fully_break;
      } 
      if(entry->attributes != 0x10) {
         free(nodename);
         break;
      }
      fat_file_read_with_handle(file, *entry, 512, buffer);
   }
success_break:
   memcpy(retent, entry, sizeof(FatEntry_t));
   status = 0;
fully_break:
   free(buffer);
   free(result);
   return status;
}

int fat_file_read(Vfs_t* file, uint32_t offset, uint32_t size, char* ptr) {
   FatEntry_t fe;
   if(_fat_get_filehandle(file, &fe)) 
      return 1;
   fat_file_read_with_handle(file, fe, size, ptr);
   return 0;
}

int fat_read_dir(Vfs_t* dev, uint32_t index, VfsDirent_t* dirent) {
   if(index > 16) {
      dirent->name[0] = '\0';
      return 1;
   }
   FatBpB_t fatbpb;
   int a = _parse_bpb(dev->master, &fatbpb);
   uint8_t* buffer = malloc(512);

   uint32_t root_lba = fatbpb.NumberOfFats * fatbpb.SectorsPerFat + fatbpb.ReservedSectors;
   vread(dev->master, root_lba, 1, buffer);

   int count = 0;
   for(int i=0; i<16; ++i) {
      int idx=i*32;
      FatEntry_t *entry = (void*)(buffer + idx);

      if(!entry->starting_cluster) continue;
      if(!(count++ >= index))
         continue;
      memcpy(dirent->name, entry->filename, 8);
      return 0;
   }

   free(buffer);
   return 1;
}

int fat_find_dir(Vfs_t* fatfs, char* path, Vfs_t* target) {
   if(fatfs->flag != VFS_FILESYSTEM)
      return -2;
   fat_file_create(target, path, fatfs->master);

   FatEntry_t fe;
   if(_fat_get_filehandle(target, &fe))
      return 1;
   _fat_node_create(target, path, fatfs->master, &fe);
   return 0;
}

int fat_dir_readdir(Vfs_t* dir, uint32_t index, VfsDirent_t* out) {
   if(dir->flag != VFS_DIRECTORY)
      return -2;
   FatEntry_t fe;
   if(_fat_get_filehandle(dir, &fe))
      return 1;

   char* buffer = malloc(512);
   fat_file_read_with_handle(dir, fe, 512, buffer);

   int idx=index*32;
   FatEntry_t *entry = (void*)(buffer + idx);
   memcpy(out->name, entry->filename, 8);

   free(buffer);
   return 0;
}

int fat_dir_finddir(Vfs_t* dir, char* filename, Vfs_t* target) {
   if(dir->flag != VFS_DIRECTORY)
      return -2;

   FatEntry_t fe;
   if(!_fat_get_filehandle(dir, &fe))
      return 1;

   char* buffer = malloc(512);
   fat_file_read_with_handle(dir, fe, 512, buffer);

   FatEntry_t *entry = _fat_get_filehandle_buffer(filename, buffer);
   if(!entry) {
      free(buffer); 
      return 1;
   }
   _fat_node_create(target, entry->filename, dir->master, entry);
   free(buffer); 
   return 0;
}

