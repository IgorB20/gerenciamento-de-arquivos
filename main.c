#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"

/*
    3.1. Modelagem da Estrutura de Diretórios
    3.2. Representação e Gerenciamento de Arquivos e Metadados
    Criar uma estrutura struct em C para representar um File Control
Block (FCB) para cada arquivo. Este FCB deve armazenar atributos
como: nome (string), tamanho (inteiro), tipo (enum ou inteiro para
dados: numérico, caractere, binário; programa), data de
criação/modificação/acesso (time_t), um ID único (simulando um
inode) (inteiro), e as permissões de acesso (inteiro ou bitmask).
o Implementar operações básicas como criar arquivo (touch), escrever
conteúdo (echo > ou similar), ler conteúdo (cat ou similar), copiar
(cp), mover/renomear (mv) e excluir (rm) – todas realizadas
internamente na estrutura de dados simulada, não no sistema de
arquivos real
    3.3. Controle de Acesso e Permissões
    3.4. Simulação de Alocação de Blocos
*/

#define MAX_SUBDIRS 5
#define MAX_FILES 50

typedef struct FCB
{
    int id;
    char name[20];
    //int tamanho; // bytes

    time_t created;

} FCB;

typedef struct Dir
{
    char name[10];
    struct Dir *subdirs[MAX_SUBDIRS];
    struct FCB *files[MAX_FILES];
    int nextDirIndex;
    int nextFCBIndex;
    int nextFCBid;
    struct Dir *parent;
} Dir;

/*
    retorna -1 se na achar
    retorna o index do dir se achar
*/
int findDir(char name [10], Dir * dir){
    int foundIndex = -1;
    for (int i = 0; i < MAX_SUBDIRS; i++)
    {
        if (dir->subdirs[i] != NULL && strcmp(dir->subdirs[i]->name, name) == 0)
        {
            foundIndex = i;
            break;
        }
    }
    return foundIndex;
}

/*
    retorna -1 se na achar
    retorna o index do dir se achar
*/
int findFile(char name [10], Dir *dir)
{
    int foundIndex = -1;
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (dir->files[i] != NULL && strcmp(dir->files[i]->name, name) == 0)
        {
            foundIndex = i;
            break;
        }
    }
    return foundIndex;
}

int main() {
    Dir root;
    strcpy(root.name, "root");
    root.parent = NULL;

    for (int i = 0; i < MAX_SUBDIRS; i++)
    {
        root.subdirs[i] = NULL;
    }
    for (int i = 0; i < MAX_FILES; i++)
    {
        root.files[i] = NULL;
    }
    root.nextDirIndex = 0;
    root.nextFCBid = 1;

    Dir *currentDir = &root;

    char linha[200];

    while(1){
        printf("/%s:", currentDir->name);
        // Lê as duas palavras separadas por espaço
        if (fgets(linha, sizeof linha, stdin) != NULL) {

             char *comando = strtok(linha, " \t\n");
             char *nome1   = strtok(NULL, " \t\n");
             char *nome2   = strtok(NULL, " \t\n");

        if (comando == NULL) {
            continue;
        // COMANDOS DIRETORIOS
        } else if (strcmp(comando, "rmdir") == 0 && nome1 != NULL)
        {
            // procurar diretorio
            int foundIndex = findDir(nome1, currentDir);

            if(foundIndex == -1){
                printf("Diretório não encontrado.");
                continue;
            }

            free(currentDir->subdirs[foundIndex]);
            currentDir->subdirs[foundIndex] = NULL;

            if(foundIndex < MAX_SUBDIRS - 1){
                // todos os indices depois do diretorio encontrado tem que andar 1 pra tras.
                for (int i = foundIndex + 1; i < MAX_SUBDIRS; i++)
                {
                    if (currentDir->subdirs[i] == NULL)
                        break;

                    currentDir->subdirs[i - 1] = currentDir->subdirs[i];
                    currentDir->subdirs[i] = NULL;
                }
            }

            

            currentDir->nextDirIndex--;

            continue;
        }

         else if (strcmp(comando, "mkdir") == 0 && nome1 != NULL)
        {
            if (currentDir->nextDirIndex >= MAX_SUBDIRS)
            {
                printf("Diretório cheio. ");
                printf("Limite de %d pastas atingido.\n", MAX_SUBDIRS);
                continue;
            }

            Dir *newDir = (Dir *)malloc(sizeof(Dir)); // alocação dinâmica de memória
            strcpy(newDir->name, nome1);
            newDir->nextDirIndex = 0;
            newDir->parent = currentDir;
            for (int i = 0; i < MAX_SUBDIRS; i++)
            {
                newDir->subdirs[i] = NULL;
            }
            for (int i = 0; i < MAX_FILES; i++)
            {
                newDir->files[i] = NULL;
            }

            currentDir->subdirs[currentDir->nextDirIndex] = newDir; // Adiciona o novo diretório à raiz
            currentDir->nextDirIndex++;
            continue;
        }

         else if (strcmp(comando, "ls") == 0)
        {
            for (int i = 0; i < MAX_SUBDIRS; i++)
            {
                if (currentDir->subdirs[i] != NULL)
                {
                    printf("%s ", currentDir->subdirs[i]->name);
                }
            }
            printf("\n");
            for (int i = 0; i < MAX_FILES; i++)
            {
                if (currentDir->files[i] != NULL)
                {
                    printf("%s ", currentDir->files[i]->name);
                }
            }
            printf("\n");
            continue;
        }

         else if (strcmp(comando, "mv") == 0 && nome1 != NULL && nome2 != NULL)
        {

            // busca primeiro diretorio
            int foundIndex1 = findDir(nome1, currentDir);
            int foundFileIndex1 = -1;

            if (foundIndex1 == -1)
                foundFileIndex1 = findFile(nome1, currentDir);
            
            if( foundIndex1 == -1 && foundFileIndex1 == -1){
                printf("Diretório/arquivo não encontrado.\n");
                continue;
            }

            // busca segundo diretorio
            int foundIndex2 = findDir(nome2, currentDir);
            int foundFileIndex2 = -1;
            
            if (foundIndex2 == -1)
                foundFileIndex2 = findFile(nome2, currentDir);

            if(foundIndex2 == -1 && foundFileIndex2 == -1)
            {
                if(foundIndex1 > -1)  // renomear diretorio
                    strcpy(currentDir->subdirs[foundIndex1]->name, nome2);

                if (foundFileIndex1 > -1) // renomear arquivo
                    strcpy(currentDir->files[foundFileIndex1]->name, nome2);
                    
            }

            if (foundIndex2 > -1) //mover
            {
                //mover pasta nome1 pra dentro de nome2
                Dir *dirToMove = currentDir->subdirs[foundIndex1];
                Dir *dirToReceive = currentDir->subdirs[foundIndex2];

                if (dirToReceive->nextDirIndex >= MAX_SUBDIRS)
                {
                    printf("Diretório cheio. ");
                    continue;
                }

                dirToReceive->subdirs[dirToReceive->nextDirIndex] = dirToMove;
                currentDir->subdirs[foundIndex1] = NULL;

                if (foundIndex1 < MAX_SUBDIRS - 1)
                {
                    // todos os indices depois do diretorio removido tem que andar 1 pra tras.
                    for (int i = foundIndex1 + 1; i < MAX_SUBDIRS; i++)
                    {
                        if (currentDir->subdirs[i] == NULL)
                            break;

                        currentDir->subdirs[i - 1] = currentDir->subdirs[i];
                        currentDir->subdirs[i] = NULL;
                    }
                }

                dirToReceive->nextDirIndex++;
                currentDir->nextDirIndex--;
            }

            continue;
        }

         else if (strcmp(comando, "cd") == 0 && nome1 != NULL)
        {
            if (strcmp(nome1, "..") == 0 && currentDir->parent != NULL)
            {
                currentDir = currentDir->parent;
            }
            else
            {
                int foundIndex = findDir(nome1, currentDir);

                if (foundIndex == -1){
                    printf("Diretório não encontrado.\n");
                    continue;
                }

                currentDir = currentDir->subdirs[foundIndex];      
            }

            continue;
        }
        // ======================

        // COMANDOS ARQUIVOS
        else if (strcmp(comando, "touch") == 0 && nome1 != NULL){
            //todo - verificar nome do arquivo

            FCB * fcb = (FCB *)(malloc(sizeof(FCB)));
            strcpy(fcb->name, nome1);
            fcb->created = time(NULL);
            fcb->id = currentDir->nextFCBid;

            currentDir->files[currentDir->nextFCBIndex] = fcb;
            currentDir->nextFCBIndex++;
            currentDir->nextFCBid++;
            continue;
        }

        else if (strcmp(comando, "rm") == 0 && nome1 != NULL)
        {
            // procurar arquivo
            int foundIndex = -1;
            for (int i = 0; i < MAX_FILES; i++)
            {
                if (strcmp(currentDir->files[i]->name, nome1) == 0)
                {
                    foundIndex = i;
                    break;
                }
            }

            if (foundIndex == -1)
            {
                printf("Arquivo não encontrado.");
                continue;
            }

            free(currentDir->files[foundIndex]);
            currentDir->files[foundIndex] = NULL;

            if (foundIndex < MAX_FILES - 1)
            {
                // todos os indices depois do file encontrado tem que andar 1 pra tras.
                for (int i = foundIndex + 1; i < MAX_FILES; i++)
                {
                    if (currentDir->files[i] == NULL)
                        break;

                    currentDir->files[i - 1] = currentDir->files[i];
                    currentDir->files[i] = NULL;
                }
            }

            currentDir->nextFCBIndex--;
            printf("Arquivo removido \n");
            continue;
        }
        else {

        }
    }
    }

    return 0;
}
