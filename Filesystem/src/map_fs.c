/*
 * map_fs.c
 *
 *  Created on: 02/11/2013
 *      Author: utnso
 */

#include "map_fs.h"
#include <sys/stat.h>
#define is ==
#define BAD_MAP (void*)(-1)

#ifndef O_RDWR
#define O_RDWR 2
#endif

#define BLOCK_SIZE 4096

char *map_filesys(char* path){

	int32_t fs;
	char *addr;

	if((fs = open(path,O_RDWR)) is -1)
		perror("Bad file open try");

	struct stat stats_fs;

	stat(path,&stats_fs);
	FS_SIZE = stats_fs.st_size;

	addr = mmap(0,FS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fs, 0);

	if(addr == MAP_FAILED){
		perror("BAD MAP!");
		exit(1);
	}

	/*posix_madvise(mapped_fs,stats_fs.st_size,MADV_RANDOM);*/

	return addr;

}

/* int32_t Unmap_FS(void* mapped_fs){
	if(munmap(mapped_fs,FS_SIZE) != -1)
		return 0;
	else
		return -1;
} */


char *blk_init_filesys(char* path, uint32_t size_bm_enblok){

printf("Tamaño en bloques del bmap: %d", size_bm_enblok);

  char buff[BLOCK_SIZE * size_bm_enblok];
FILE * arch;
long int comienzo, actual;
arch = fopen( path, "r" );

if( arch )
    printf( "FYLESYS/MAPFS: Archivo abierto correctamente: ", path );
  else
  {
	  printf( "FYLESYS/MAPFS: Problema al abrir el archivo: ", path );
    return 1;
  }

if( (comienzo=ftell( arch )) < 0 )   printf( "ERROR: ftell no ha funcionado\n" );
  else    printf( "Posición del fichero: %d\n", comienzo );

fseek(arch, BLOCK_SIZE ,SEEK_SET);

actual=ftell( arch);

printf( "Posición del fichero luego del SEEK: %d\n", actual );

fread(buff,BLOCK_SIZE * size_bm_enblok,1,arch);

actual=ftell( arch);

printf( "Posición del fichero luego de leer el BITMAP: %d\n", actual );


return buff;

}





