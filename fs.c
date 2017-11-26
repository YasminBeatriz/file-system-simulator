/*
 * RSFS - Really Simple File System
 *
 * Copyright © 2010 Gustavo Maciel Dias Vieira
 * Copyright © 2010 Rodrigo Rocco Barbieri
 *
 * This file is part of RSFS.
 *
 * RSFS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define CLUSTERSIZE 4096

unsigned short fat[65536];

typedef struct {
       char used;
       char name[25];
       unsigned short first_block;
       int size;
} dir_entry;

dir_entry dir[128];


int fs_init() {
    int sector, agrupamento, buffer_offset;
    char *buffer = (char *) malloc (sizeof(fat));
    int isFAT = VERDADEIRO, isDir = VERDADEIRO;
    int i;
  
    //Le a tabela FAT e o diretorio do disco em um buffer
    buffer_offset = 0;
    for(agrupamento = 0; agrupamento < 33; agrupamento++)
    {
        sector = 0;
        while(sector < 8)
        {
            bl_read(agrupamento * 8 + sector, &buffer[buffer_offset]);
            sector++;
            buffer_offset = buffer_offset + 512;
        }
    }

    //Vê se as 32 posicoes da tabela FAT referenciada pelo buffer tem o codigo FAT  
    buffer_offset = 0;
    for(agrupamento = 0; agrupamento < 32; agrupamento++)
    {
        if(buffer[buffer_offset] != FAT)
        {
            isFAT = FALSO;
            break;
        }
        else
            buffer_offset = buffer_offset + 2;
    }

    //Vê se a 33a posicao da tabela FAT referenciada pelo buffer tem o codigo DIRETORIO
    if(buffer[buffer_offset] != DIRETORIO)
    {
        isDir = FALSO;
    }

    if(!isFAT || !isDir)
    {
        fs_format();
    }
    else
    {
        // Carrega a tabela FAT e o diretorio

        // FAT já está em buffer
        buffer_offset = 0;
        for(i = 0; i < 65536; i++)
        {
            fat[i] = buffer[buffer_offset];
            buffer_offset = buffer_offset + 2;
        }

        // buffer pega o diretorio que está no disco no agrupamento 33
        buffer = (char *) malloc (sizeof(dir));
        buffer_offset = 0;
        sector = 0;
        while(sector < 8)
        {
            bl_read(32 * 8 + sector, &buffer[buffer_offset]);
            sector++;
        }
    
        // copia buffer para o vetor de diretorios
        memcpy(&dir, buffer, 128);
    }

    return 1;
}

int fs_format() {
    int i, sector, agrupamento, buffer_offset;
    char *buffer;

    //inicializa a tabela FAT
    for(i = 0; i < 32; i++)
        fat[i] = FAT;
    
    for(i = 32; i < 33; i++)
        fat[i] = DIRETORIO;

    for(i = 33; i < 65536; i++)
        fat[i] = LIVRE;

    // Escreve a FAT no disco
    buffer = (char *) fat;
    buffer_offset = 0;

    for(agrupamento = 0; agrupamento < 32; agrupamento++)
    {
        sector = 0;
        while(sector < 8)
        {
            bl_write(agrupamento * 8 + sector, &buffer[buffer_offset]);
            sector++;
            buffer_offset = buffer_offset + 512;
        }
    }

    //Zera o diretorio
    for(i = 0; i < 128; i++)
    {
        dir[i].used = FALSO;
    }

    //Escreve o diretorio no disco
    buffer = (char *) dir;
    buffer_offset = 0;
    agrupamento = 32;
    sector = 0;

    while(sector < 8)
    {
        bl_write(agrupamento * 8 + sector, &buffer[buffer_offset]);
        sector++;
        buffer_offset = buffer_offset + 512;
    }
    
    return 1;
}

int fs_free() {
    int livres = 0;
    int i;

    for(i = 33; i < 65536; i++)
    {
        if(fat[i] == LIVRE)
            livres++;
    }

    return livres * CLUSTERSIZE;
}

int fs_list(char *buffer, int size) {
  printf("Função não implementada: fs_list\n");
  return 0;
}

int fs_create(char* file_name) {
    int i, j; 
    int fat_livre = FALSO, dir_livre = FALSO;
    
    for(j = 0; j < 128; j++)
    {
        if(strcmp(dir[j].name, file_name) == 0)
        {
            printf("Arquivo com mesmo nome já existe.\n");
            return 0;
        }
    }

    // Faz varredura na FAT em busca de uma posicao marcada como LIVRE
    for(i = 33; i < 65536; i++)
    {
        if(fat[i] == LIVRE)
        {
            fat_livre = VERDADEIRO;
            break;
        }
    }
    
    if(fat_livre)
    {
        for(j = 0; j < 128; j++)
        {
            if(dir[j].used == FALSO)
            {
                dir_livre = VERDADEIRO;
                // Atualiza a FAT
                fat[i] = ULTIMO;

                // Atualiza a entrada no vetor de arquivos
                dir[j].used = VERDADEIRO;
                dir[j].first_block = i;
                dir[j].size = 0;
                strcpy(dir[j].name, file_name);

                return 1;
            }   
        }

        if(!dir_livre)
            printf("Disco cheio!");
    }
    else
    {
        printf("Disco cheio!");
    }
    
    return 0;
}

int fs_remove(char *file_name) {
  printf("Função não implementada: fs_remove\n");
  return 0;
}

int fs_open(char *file_name, int mode) {
  printf("Função não implementada: fs_open\n");
  return -1;
}

int fs_close(int file)  {
  printf("Função não implementada: fs_close\n");
  return 0;
}

int fs_write(char *buffer, int size, int file) {
  printf("Função não implementada: fs_write\n");
  return -1;
}

int fs_read(char *buffer, int size, int file) {
  printf("Função não implementada: fs_read\n");
  return -1;
}

