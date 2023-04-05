#pragma once
#include <stdint.h>

#define VFS_NAME_MAX (55)
#define VFS_PATH_MAX (255)
#define VFS_MOUNT_MAX_COUNT (15)

typedef enum vfs_flag_t {
   VFS_INVALID      =   0x0,

   VFS_FILE         =   0x1,
   VFS_DIRECTORY    =   0x2,
   VFS_CHAR_DEV     =   0x3,
   VFS_BLOCK_DEV    =   0x4,
   VFS_PIPE         =   0x5,
   VFS_SYMLINK      =   0x6,
   VFS_MOUNTPOINT   =   0x8,
   VFS_FILESYSTEM   =   0x10,
} VfsFlag_t;

typedef struct vfs_dirent_t {
   char name[VFS_NAME_MAX];
   uint32_t inode_num;
} VfsDirent_t;

struct vfs_t;
//node, offset, size, ptr
typedef int (*vfs_read_callback)(struct vfs_t*, uint32_t, uint32_t, char* );
typedef int (*vfs_write_callback)(struct vfs_t*, uint32_t, uint32_t, char* );
typedef int (*vfs_open_callback)(struct vfs_t*);
typedef int (*vfs_close_callback)(struct vfs_t*);
typedef int (*vfs_readdir_callback)(struct vfs_t*, uint32_t, struct vfs_dirent_t*);
typedef int (*vfs_finddir_callback)(struct vfs_t*, char*, struct vfs_t**);

typedef struct vfs_t {
   char name[VFS_NAME_MAX];
   VfsFlag_t flag;

   vfs_read_callback read;
   vfs_write_callback write;

   vfs_open_callback open;
   vfs_close_callback close;

   vfs_readdir_callback read_dir;
   vfs_finddir_callback find_dir;

   struct vfs_t* master;
} Vfs_t;

typedef struct mount_info_t {
   char* path;
   struct vfs_t* device;
} MountInfo_t;

Vfs_t* fopen(char* path);
int fclose(Vfs_t*);

//node, offset, size, ptr
int vread(Vfs_t*, uint32_t, uint32_t, char*);
int vwrite(Vfs_t*, uint32_t, uint32_t, char*);
int vopen(Vfs_t*);
int vclose(Vfs_t*);
int vread_dir(Vfs_t*, uint32_t, VfsDirent_t*);
int vfind_dir(Vfs_t*, char*, Vfs_t**);

int vmount_device(Vfs_t*, char*, uint32_t);

Vfs_t vfs_node_new();
int vfs_init();

