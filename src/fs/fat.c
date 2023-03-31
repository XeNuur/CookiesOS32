#include "fat.h"
#include "../kernel/device.h"
#include "../kernel/panic.h"
#include "../kernel/framebuffer.h"
#include "../kernel/kheap.h"
#include "vfs.h"

#include <string.h>

#define FAT_MAX_SUBNAME (16)

FatBpB_t *_parse_bpb(Vfs_t *dev) {
   char* buffer = malloc(512);
   vread(dev, 0, 1, buffer);

   FatBpB_t *fatbpb = malloc(sizeof(FatBpB_t));
   memcpy(fatbpb, buffer+3, sizeof(FatBpB_t));

   free(buffer);
   return fatbpb;
}

int fat_check(Vfs_t* device) {
   if(device->flag != VFS_BLOCK_DEV || !device->read) {
      return 0;
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
   return 1;
}

int fat_file_create(Vfs_t* node, char* path, Vfs_t* device) {
   *node = vfs_node_new();
   memcpy(node->name, path, VFS_NAME_MAX);

   node->read = fat_file_read;
   node->master = device;
   node->flag = VFS_FILE;
}

FatEntry_t *_fat_get_filehandle_rootdir(Vfs_t* dev, char* path) {
   FatBpB_t* fatbpb = _parse_bpb(dev);
   uint8_t* buffer = malloc(512);

   uint32_t root_lba = fatbpb->NumberOfFats * fatbpb->SectorsPerFat + fatbpb->ReservedSectors;
   vread(dev, root_lba, 1, buffer);

   FatEntry_t *status = 0;
   int i=0;
   while(i < 512) {
      FatEntry_t *entry = (void*)(buffer + i);
      if(memcmp(entry->filename, path, strlen(path)) == 0) {
         status = malloc(sizeof(FatEntry_t));
         memcpy(status, entry, sizeof(FatEntry_t));
         break;
      }
      i += 32;
   }
   free(fatbpb); free(buffer);
   return status;
}

int fat_file_read_with_handle(Vfs_t* file, FatEntry_t *handle, uint32_t size, char* ptr) {
   if(!file->flag == VFS_FILE)
      return 0;
   if(!handle) return 0;

   FatBpB_t* fatbpb = _parse_bpb(file->master);
   uint32_t root_lba = fatbpb->NumberOfFats * fatbpb->SectorsPerFat + fatbpb->ReservedSectors;
   uint8_t* buffer = malloc(512);
   uint16_t current_cluster = handle->starting_cluster;

   uint32_t index = 0;

   for(;;) {
      uint32_t data_sec = (root_lba+32);
      uint32_t lba_ctx = data_sec + (current_cluster-2) * fatbpb->SectorsPerCluster;
      vread(file->master, lba_ctx, 1, buffer);

      for(uint32_t i=0; i<512; i++) {
         ptr[index] = ((char*)buffer)[i];
         index++;
         if(index > size)
            break;
      }

      uint32_t frist_fat = fatbpb->SectorsPerFat+fatbpb->ReservedSectors;
      uint32_t fat_offset = current_cluster * 2;
      uint32_t fat_sector = frist_fat + (fat_offset / fatbpb->BytesPerSector);
      uint32_t ent_offset = fat_offset % fatbpb->BytesPerSector;
      vread(file->master, fat_sector, 1, buffer);
      current_cluster = *(uint16_t*)&buffer[ent_offset];

      if(current_cluster >= FAT16_EOF)
         break;
   }
   free(fatbpb); free(buffer);
   return 1;
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

FatEntry_t* _fat_get_filehandle(Vfs_t* file, char* path) {
   char *nodename = strtok(path, '/');
   char* buffer = malloc(512);

   FatEntry_t* result = _fat_get_filehandle_rootdir(file->master, nodename);
   if(!result) return 0;
   FatEntry_t *entry = result;
   FatEntry_t *retval = 0;
   
   if(result->attributes != 0x10) {
      free(nodename);
      goto success_break;
   }
   fat_file_read_with_handle(file, result, 512, buffer);

   int found = 0;
   while(nodename != NULL) {
      free(nodename);
      nodename = strtok(0, '/');

      uint32_t i=0;
      while(i < 512) {
         entry = (void*)(buffer + i);
            
         if(memcmp(entry->filename, nodename, strlen(nodename)) == 0) {
            found = 1;
            if(entry->attributes != 0x10) {
               free(nodename);
               goto success_break;
            }
            fat_file_read_with_handle(file, entry, 512, buffer);
         }
         i+=32;
      }
      if(!found) {
         free(nodename);
         goto fully_break;
      }
      found = 0;
   }
success_break:
   retval = malloc(sizeof(FatEntry_t));
   memcpy(retval, entry, sizeof(FatEntry_t));
fully_break:
   free(buffer);
   free(result);
   return retval;
}

int fat_file_read(Vfs_t* file, uint32_t offset, uint32_t size, char* ptr) {
   FatEntry_t* fe = _fat_get_filehandle(file, file->name);
   if(!fe) return 0;
   fat_file_read_with_handle(file, fe, size, ptr);
   free(fe);
   return 1;
}

int fat_read_dir(Vfs_t* dev, uint32_t index, VfsDirent_t* dirent) {
   FatBpB_t* fatbpb = _parse_bpb(dev);
   uint8_t* buffer = malloc(512);

   uint32_t root_lba = fatbpb->NumberOfFats * fatbpb->SectorsPerFat + fatbpb->ReservedSectors;
   vread(dev, root_lba, 1, buffer);

   int idx=index*32;
   FatEntry_t *entry = (void*)(buffer + idx);
   memcpy(dirent->name, entry->filename, 8);

   free(fatbpb); free(buffer);
   return 0;
}

int fat_find_dir(Vfs_t* fatfs, char* path, Vfs_t* target) {
   if(fatfs->flag != VFS_FILESYSTEM)
      return 0;
   //TODO: check if file exist
   fat_file_create(target, path, fatfs->master);
   FatEntry_t* fe = _fat_get_filehandle(target, path);
   if(!fe) return 0;
   free(fe);
   return 1;
}

