#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

void syncronize_directory(char *original_folder, char *backup_folder);
int create_backup_file(char *original_folder, char *backup_folder);

int main(int argc, char **argv)
{
	char *original_folder = argv[1];
	char *backup_folder = argv[2];

    while (1) {
		syncronize_directory(original_folder, backup_folder);
		sleep(5);
	}

	return 0;
}

void syncronize_directory(char* original_folder, char* backup_folder)
{
	int i;

	struct stat st = {0};

	if(stat(backup_folder, &st) == -1)
	    mkdir(backup_folder, 0700);

	struct dirent **namelistOrigem;
	struct dirent **namelistDestino;
    struct stat arquivo, arquivo2;

	int nOrigem = scandir(original_folder, &namelistOrigem, NULL, alphasort);
	int nDestino = scandir(backup_folder, &namelistDestino, NULL, alphasort);

    if (nOrigem < 0)
        perror("scandir()");
    else
    {
        for (i = 2; i < nOrigem; i++) // cada arquivo da pasta de original_folder
        {
        	char *fileNameOrigem = (char*) malloc(200*sizeof(char));
        	fileNameOrigem[0] = '\0';
        	strcat(fileNameOrigem, original_folder);
            
    		if ((namelistOrigem[i]->d_type == DT_DIR))
        	{
        		strcat(fileNameOrigem, "/");
        		strcat(fileNameOrigem, namelistOrigem[i]->d_name);

				char *aux2 = (char*) malloc(100*sizeof(char));
				aux2[0] = '\0';
				strcat(aux2, backup_folder);
        		strcat(aux2, "/");
				strcat(aux2, namelistOrigem[i]->d_name);

				struct stat st = {0};
				if (stat(aux2, &st) == -1) // se a pasta nao existe, criar
				    mkdir(aux2, 0700);

				syncronize_directory(fileNameOrigem, aux2); // chamada recursiva pra subpasta
        	}
        	else
        	{
        		strcat(fileNameOrigem, "/");
        		strcat(fileNameOrigem, namelistOrigem[i]->d_name);
			    if (stat(fileNameOrigem, &arquivo) == -1)
			    	printf("ERRO\n");

    			int j;
    			for (j = 2; j < nDestino; j++) // procurar arquivo da pasta de backup_folder
    			{
    				char *fileNameDestino = (char*) malloc(200*sizeof(char));
		        	fileNameDestino[0] = '\0';
		        	strcat(fileNameDestino, backup_folder);
		        	if (strcmp(backup_folder, ".") == 0) strcat(fileNameDestino, "/");
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

		    				strcat(aux2, backup_folder);
			        		strcat(aux2, "/");
							strcat(aux2, namelistOrigem[i]->d_name);

		    				create_backup_file(fileNameOrigem, aux2);
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
    				strcat(aux2, backup_folder);
					struct stat st = {0};
    				if (stat(aux2, &st) == -1) {
					    mkdir(aux2, 0700);
					}
        			strcat(aux2, "/");
    				strcat(aux2, namelistOrigem[i]->d_name);

    				create_backup_file(fileNameOrigem, aux2);
    			}
			}
            free(namelistOrigem[i]);
        }
        free(namelistOrigem);
    }
}

int create_backup_file(char *original_folder, char *backup_folder)
{
	int fd_o, fd_d;

	fd_o = open(original_folder, O_RDONLY);
	if (fd_o == -1) {
		perror("open()");
		return 0;
	}
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	fd_d = open(backup_folder, O_CREAT | O_RDWR | O_TRUNC, mode);
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
	}while(nr > 0);
	close(fd_o);
	close(fd_d);
}