// Pedro Spoljaric Gomes 112344
// Tamires Beatriz da Silva Lucena 111866

#define _USE_32BIT_TIME_T 1

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include "sha1.c"

void Atualizar(char *origem, char *destino);
void print_stat(struct stat buf);
int cp(char *origem, char *destino);
void hexResult(char *w, int tamanho);

#define print_type(t) printf(" %s ", #t);

void print_type_name(unsigned char type) {
	switch(type) {
		case DT_DIR: print_type(DT_DIR); break;
		case DT_REG: print_type(DT_REG); break;
	}
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("comando: ./dropbox pastaOriginal pastaBackup\n");
		return 0;
	}

	char *origem = argv[1];
	char *destino = argv[2];

    int i;

    while (1)
    {
		sleep(5);

		Atualizar(origem, destino);
	}

	return 0;
}

char* hash(char *w, int tamanho)
{
    char hashHex[21];
    char *hexResult = (char*) malloc(41*sizeof(char));
    bzero(hexResult, 41);

	SHA1(hashHex, w, tamanho);

    int i;
    for (i = 0; i < 20; i++)
        sprintf( ( hexResult + (2*i)), "%02x", hashHex[i]&0xff);

    return hexResult;
}

void Atualizar(char *origem, char *destino)
{
	int i;

	struct stat st = {0};
	if (stat(destino, &st) == -1) {
	    mkdir(destino, 0700);
	}

	struct dirent **namelistOrigem;
	struct dirent **namelistDestino;
    struct stat arquivo, arquivo2;

	int nOrigem = scandir(origem, &namelistOrigem, NULL, alphasort);
	int nDestino = scandir(destino, &namelistDestino, NULL, alphasort);

    if (nOrigem < 0)
        perror("scandir()");
    else
    {
        for (i = 2; i < nOrigem; i++) // cada arquivo da pasta de origem
        {
        	char *fileNameOrigem = (char*) malloc(200*sizeof(char));
        	fileNameOrigem[0] = '\0';
        	strcat(fileNameOrigem, origem);
        	if (strcmp(origem, ".") == 0) strcat(fileNameOrigem, "/");


    		if ((namelistOrigem[i]->d_type == DT_DIR))
        	{
        		strcat(fileNameOrigem, "/");
        		strcat(fileNameOrigem, namelistOrigem[i]->d_name);

				char *aux2 = (char*) malloc(100*sizeof(char));
				aux2[0] = '\0';
				strcat(aux2, destino);

        		strcat(aux2, "/");
				strcat(aux2, namelistOrigem[i]->d_name);

				struct stat st = {0};
				if (stat(aux2, &st) == -1) // se a pasta nao existe, criar
				    mkdir(aux2, 0700);

				Atualizar(fileNameOrigem, aux2); // chamada recursiva pra subpasta
        	}
        	else
        	{
        		strcat(fileNameOrigem, "/");
        		strcat(fileNameOrigem, namelistOrigem[i]->d_name);
			    if (stat(fileNameOrigem, &arquivo) == -1)
			    	printf("ERRO\n");

    			int j;
    			for (j = 2; j < nDestino; j++) // procurar arquivo da pasta de destino
    			{
    				char *fileNameDestino = (char*) malloc(200*sizeof(char));
		        	fileNameDestino[0] = '\0';
		        	strcat(fileNameDestino, destino);
		        	if (strcmp(destino, ".") == 0) strcat(fileNameDestino, "/");
        			strcat(fileNameDestino, "/");
		        	strcat(fileNameDestino, namelistDestino[j]->d_name);

				    if (stat(fileNameDestino, &arquivo2) == -1)
				    	printf("ERRO\n");

				    if (strcmp(fileNameOrigem, fileNameOrigem) == 0)
				    {
						if (difftime(arquivo.st_mtime, arquivo2.st_mtime) > 0)
	        			{
	        				printf("Atualizando arquivo %s\n", fileNameOrigem);

		    				char *aux2 = (char*) malloc(50*sizeof(char));
		    				aux2[0] = '\0';
		    				strcat(aux2, destino);

			        		if (strcmp(destino, ".") == 0) strcat(aux2, "/");
							strcat(aux2, namelistOrigem[i]->d_name);
							struct stat st = {0};
		    				if (stat(aux2, &st) == -1) {
							    mkdir(aux2, 0700);
							}

		    				cp(fileNameOrigem, fileNameDestino);
	        			}
	        			else
	        			{
	        				printf("Arquivo ja atualizado %s\n", fileNameOrigem);
	        			}
	        			break;
				    }
    			}
    			if (j == nDestino)
    			{
    				printf("Novo backup %s\n", fileNameOrigem);
    				char *aux2 = (char*) malloc(50*sizeof(char));
    				aux2[0] = '\0';
    				strcat(aux2, destino);
					struct stat st = {0};
    				if (stat(aux2, &st) == -1) {
					    mkdir(aux2, 0700);
					}
        			strcat(aux2, "/");
    				strcat(aux2, namelistOrigem[i]->d_name);

    				cp(fileNameOrigem, aux2);
    			}
			}
            free(namelistOrigem[i]);
        }
        free(namelistOrigem);
    }
}

void print_stat(struct stat buf){
	printf ("\n\t inode: %lld \
		 \n\t size:   %lld \
		 \n\t block size: %lld \
		 \n\t n blocks: %lld \
		 \n\t acesso: %s \
	 modif:  %s \
	 status: %s ",
		(long long) buf.st_ino,
		(long long) buf.st_size,
		(long long) buf.st_blksize,
		(long long) buf.st_blocks,
		ctime(&buf.st_atime),
		ctime(&buf.st_mtime),
		ctime(&buf.st_ctime));
}


int cp(char *origem, char *destino)
{
	int fd_o, fd_d;
	fd_o = open(origem, O_RDONLY);
	if (fd_o == -1) {
		perror("open()");
		return 0;
	}
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	fd_d = open(destino, O_CREAT | O_RDWR | O_TRUNC, mode);
	if(fd_d == -1) {
		perror("open()");
		close(fd_o);
		return 0;
	}
#define BLOCO 4096
	int nr, ns, nw, n, nBckp;
	unsigned char buffer[BLOCO], bufferBckp[BLOCO];
	void *ptr_buff;
	do {
		nr = read(fd_o, buffer, BLOCO);
		nBckp = read(fd_d, bufferBckp, BLOCO);
		if (nr == -1) {
			perror("read()");
			close(fd_o);
			close(fd_d);
			return 0;
		}
		else if (nr > 0) {
			if (strcmp(hash(buffer, nr), hash(bufferBckp, nBckp)) != 0) // copiar somente blocos diferentes
			{
				ptr_buff = buffer;
				nw = nr;
				ns = 0;
				do {
					n = write(fd_d, ptr_buff + ns, nw);
					if (n == -1) {
						perror("write()");
	                	        	close(fd_o);
		        	                close(fd_d);
			                        return 0;
					}
					ns += n;
					nw -= n;
				} while (nw > 0);
			}
		}
	}while(nr > 0);
	close(fd_o);
	close(fd_d);
}