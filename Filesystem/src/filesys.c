/*
 ============================================================================
 Name        : filesys.c
 Author      : 
 Version     :
 Copyright   : chorizard!
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/bitarray.h>

#include "map_fs.h"
#include "grasa.h"

#define BLOCK_SIZE 4096

int main(void) {

	  struct stat st;

		int fd;
		GHeader *header;

		const char* file_name ="/home/utnso/disk.bin";

		stat(file_name, &st);

		unsigned int tam_fs = st.st_size;

		printf("Tamaño del FS: %u\n", tam_fs);

		if((fd = open (file_name, O_RDONLY))) puts(" FS ok");

		header = (GHeader*) mmap(NULL, 4096, PROT_READ, MAP_SHARED,fd,(off_t)NULL);

		if (header == MAP_FAILED) puts("ERROR EN EL MAPEO!");

		printf("\nHeader -> Contenido: %s\n",header->grasa);
		printf("Header -> Version: %u\n",header->version);
		printf("Header -> Bloque inicio: %u\n",header->blk_bitmap);
		printf(" Header -> Tamaño: %u\n",header->size_bitmap);

		t_bitarray* bit;
		int tam_bm;

		tam_bm =  (tam_fs / 4096) / 8;

		printf(" Proceso FS: Tamaño del BITMAP x calculo: %u\n", tam_bm);


		bit = bitarray_create(mmap(NULL, 4096, PROT_READ, MAP_SHARED,fd, 4096),tam_bm);

		printf(" Proceso FS: Tamaño del BITMAP x lib: %u\n", bitarray_get_max_bit(bit));

		//bool ocupado;
		int i,v;

		printf("\n");
		printf("ESTADO DEL FS: ");


		for (i=1; i<bitarray_get_max_bit(bit); i++)

				{
				v = bitarray_test_bit(bit,i);
				if (v==1) printf("1\n");

				if (v==0){printf("0\n");
				}
				}

	GFile* nodo;


	nodo = (GFile*) mmap(NULL, sizeof(GFile)* 1024, PROT_READ, MAP_SHARED,fd, BLOCK_SIZE*2);


	printf ("Estado: %d \n", nodo[0].state);
	printf ("Nombre: %s \n", nodo[0].fname);
	printf ("Padre: %d \n", nodo[0].parent_dir_block);
	printf ("Tamanio: %d \n", nodo[0].file_size);
	printf ("Fecha de modificacion: %d \n", (int)nodo[0].m_date);
	printf ("Fecha modificacion: %d \n", (int)nodo[0].c_date);

            // ejemplo
			printf ("Estado: %d \n", nodo[9].state);
			printf ("Nombre: %s \n", nodo[9].fname);
			printf ("Estado: %d \n", nodo[9].file_size);


			for (i=0;i<1024;i++){
				if (nodo[i].state == 2)
		     printf ("Nombre de Directorio: %s \n", nodo[i].fname);

			}

			uint32_t parent_aux = 0;
	         i=0;


	         for (i=0; i < 1024;i++)
			 if(nodo[i].state == 2)
		     {
			  parent_aux = nodo[i].parent_dir_block;

	            printf("Parent??: [%s]  ",nodo[parent_aux].fname);
			    printf("Nombre de Dir: [%s]  ",nodo[i].fname);
			    printf("Numero de Bloque del DIR %d, \n",i);

			                   }

			     parent_aux = 0;
			     i=0;
			     for (i=0; i < 1024;i++)
			      {
			        if((nodo[i].state != 2) && (nodo[i].state==1))
			         {
			           parent_aux = nodo[i].parent_dir_block;

	             printf("Parent??: [%s]  ",nodo[parent_aux].fname);
			     printf("Nombre de Arch: <%s>  ",nodo[i].fname);
	             printf("Numero de Bloque del ARCH %d  \n",i);

                      }
			             }
	return EXIT_SUCCESS;

}
