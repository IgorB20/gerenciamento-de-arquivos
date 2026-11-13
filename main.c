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
            int foundIndex = -1;
            for(int i = 0; i < MAX_SUBDIRS; i++){
                if(strcmp(currentDir->subdirs[i]->name, nome1) == 0){
                    foundIndex = i;
                    break;
                }
            }

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
            printf("Diretório removido \n");
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
                    printf("%s (%d) ", currentDir->files[i]->name, currentDir->files[i]->id);
                }
            }
            printf("\n");
            continue;
        }

         else if (strcmp(comando, "mv") == 0 && nome1 != NULL && nome2 != NULL)
        {          
            int found = 0;
            // busca arquivo
            for (int i = 0; i < MAX_SUBDIRS; i++)
            {

                if (currentDir->subdirs[i]->name != NULL && strcmp(currentDir->subdirs[i]->name, nome1) == 0)
                {
                    strcpy(currentDir->subdirs[i]->name, nome2);
                    found = 1;
                    break;
                }
            }
            
            if (found == 0){
                    printf("Arquivo não encontrado.\n");
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
                int found = 0;
                // busca o diretório
                for (int i = 0; i < MAX_SUBDIRS; i++)
                {

                    if (currentDir->subdirs[i]->name != NULL && strcmp(currentDir->subdirs[i]->name, nome1) == 0)
                    {
                        currentDir = currentDir->subdirs[i];
                        found = 1;
                        break;
                    }
                }

                if (found == 0)
                    printf("Diretório não encontrado.\n");
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
            // procurar diretorio
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
