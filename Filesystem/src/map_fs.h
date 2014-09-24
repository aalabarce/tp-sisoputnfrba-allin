/*
 * manage_fs.h
 *
 */

#ifndef MANAGE_FS_H_
#define MANAGE_FS_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>


#include <stdint.h>
#include <stdio.h>

uint32_t FS_SIZE;

	char* map_filesys(char* path);
	char *blk_init_filesys(char* path, uint32_t size_bm_enblok);
	/*int32_t Unmap_FS(void* mapped_fs);*/


#endif /* MANAGE_FS_H_ */
