#include "fat.h"
#include "../kernel/device.h"
#include "../kernel/panic.h"
#include "../kernel/framebuffer.h"
#include "../kernel/kheap.h"
#include "vfs.h"

#include <string.h>

FatBpB_t *fatbpb = 0;
uint32_t dev_index = 0;

void parse_bpb() {
   char* buffer = malloc(512);
   vread(devices_at(dev_index), 0, 1, buffer);
   fatbpb = (void*) buffer+3;

   term_printf("[FATFS] Oem:");
   for(uint32_t i=0;i<8;++i)
      term_printf("%c", fatbpb->OEMName[i]);
   term_printf("\n");
}

void foo() {
   uint32_t fat_n = fatbpb->NumberOfFats;
   uint32_t sec_per_fat = fatbpb->SectorsPerFat;
   uint32_t lba = fat_n * sec_per_fat + fatbpb->ReservedSectors;
   term_printf("[FATFS] rootdir lba: %x = (%x) * (%x) + (%x)\n", lba, sec_per_fat, fat_n, fatbpb->ReservedSectors);

   uint8_t* buffer = malloc(512);
   vread(devices_at(dev_index), lba, 1, buffer);

   term_printf("[FATFS] rootdir context: \n");
   for(uint32_t i=0;i<512;++i)
      term_printf("%c", buffer[i]);
   term_printf("\n");

   term_printf("[FATFS] rootdir entries: \n");
   int i=0;
   while(i < 512) {
      char* file_aatrib = buffer+i;
      FatEntry_t *entry = (void*)file_aatrib;
      if(!entry->starting_cluster)
         goto skip;

      for(int j=0; j<8; ++j)
         term_printf("%c", entry->filename[j]);
      term_printf(" - ");
      for(int j=0; j<3; ++j)
         term_printf("%c", entry->ext[j]);
      term_printf("(fc: %x)\n", entry->starting_cluster);

      term_printf("[FATFS] attemot to read file context: \n");
      uint32_t data_sec = (lba+32);
      uint32_t lba_ctx = data_sec + (entry->starting_cluster-2) * fatbpb->SectorsPerCluster;
      vread(devices_at(dev_index), lba_ctx, 1, buffer);
      term_printf("[FATFS] 1st sector: %x\n", lba_ctx);
      term_printf("[FATFS] rootdir and it's context: \n");
      for(uint32_t i=0;i<512;++i){
         if(buffer[i] == '\0')
            break;
         term_printf("%c", buffer[i]);
      }

      uint32_t frist_fat = fatbpb->SectorsPerFat+fatbpb->ReservedSectors;

      uint32_t fat_offset = entry->starting_cluster * 2;
      uint32_t fat_sector = frist_fat + (fat_offset / fatbpb->BytesPerSector);
      uint32_t ent_offset = fat_offset % fatbpb->BytesPerSector;

      term_printf("\n[FATFS] checksums at %x: \n", fat_sector);
      vread(devices_at(dev_index), fat_sector, 1, buffer);

      for(uint32_t i=0;i<512;++i)
         if(buffer[i] != 0)
            term_printf("(%x)%x ", i, buffer[i]);

      uint16_t next = ((uint16_t*)buffer)[ent_offset];
      if(next == FAT16_EOF)
         term_printf("\nEnd of File\n");
      term_printf(">>(%x)<<\n", next);


      term_printf("\n");
      skip:
      i += 32;
   }
}

void fat_init(uint32_t device_number) {
   if(devices_at(device_number)->flag != VFS_BLOCK_DEV) {
      yell("[FATFS]: Invalid device flag!");
      return;
   }
   dev_index = device_number;

   parse_bpb();
   foo();
}

