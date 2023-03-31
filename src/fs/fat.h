#pragma once
#include <stdint.h>
#include "vfs.h"

#define FAT16_EOF (0xFFF8)

typedef struct fat_bpb_t {
   uint8_t	OEMName[8];
   uint16_t	BytesPerSector;
   uint8_t	SectorsPerCluster;
   uint16_t	ReservedSectors;
   uint8_t	NumberOfFats;
   uint16_t	NumDirEntries;
   uint16_t	NumSectors;
   uint8_t	Media;
   uint16_t	SectorsPerFat;
   uint16_t	SectorsPerTrack;
   uint16_t	HeadsPerCyl;
   uint32_t	HiddenSectors;
   uint32_t	LongSectors;
} __attribute((packed)) FatBpB_t;

typedef struct fat_entry_t{
    unsigned char filename[8];
    unsigned char ext[3];
    unsigned char attributes;
    unsigned char reserved[10];
    unsigned short modify_time;
    unsigned short modify_date;
    unsigned short starting_cluster;
    unsigned long file_size;
} __attribute((packed)) FatEntry_t;

int fat_check(Vfs_t* device);
int fat_mount(Vfs_t* device); 

int fat_file_read(Vfs_t*, uint32_t, uint32_t, char*);

int fat_read_dir(Vfs_t*, uint32_t, VfsDirent_t*);
int fat_find_dir(Vfs_t*, char*, Vfs_t*);

static FatEntry_t *_fat_get_filehandle_rootdir(Vfs_t* dev, char* path);
