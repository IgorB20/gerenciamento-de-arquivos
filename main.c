#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"

/*
    3.1. Modelagem da Estrutura de Diretórios
    3.2. Representação e Gerenciamento de Arquivos e Metadados
    Criar uma estrutura struct em C para representar um File Control
Block (File) para cada arquivo. Este File deve armazenar atributos
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

typedef struct File
{
    int id;
    char name[20];
    //int tamanho; // bytes
    char content[200];

    time_t created;
    time_t modified;
    time_t accessed;

} File;

typedef struct Dir
{
    char name[10];
    struct Dir *subdirs[MAX_SUBDIRS];
    struct File *files[MAX_FILES];
    int nextDirIndex;
    int nextFileIndex;
    int nextFileid;
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

File * createFile(char name [10], Dir *dir){
    File *file = (File *)(malloc(sizeof(File)));
    strcpy(file->name, name);
    strcpy(file->content, "teste conteudo");
    file->created = time(NULL);
    file->modified = time(NULL);
    file->accessed = time(NULL);
    file->id = dir->nextFileid++;

    return file;
}

void addFileToDir(File * file, Dir * dir){
    dir->files[dir->nextFileIndex] = file;
    dir->nextFileIndex++;
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
    root.nextFileIndex = 0;
    root.nextFileid = 1;
    
    
    Dir *currentDir = &root;

    char linha[200];

    while(1){
        printf("/%s:", currentDir->name);
        // Lê as duas palavras separadas por espaço
        if (fgets(linha, sizeof linha, stdin) != NULL) {

             char *comando = strtok(linha, " \t\n");
             char *nome1 = strtok(NULL, " \t\n");
             char *nome2 = strtok(NULL, " \t\n");
             char *nome3 = strtok(NULL, " \t\n");
             char *nome4 = strtok(NULL, " \t\n");

             if (comando == NULL)
             {
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
                    printf("%s (%d) ", currentDir->files[i]->name, currentDir->files[i]->id);
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
                foundFileIndex1 = findFile(nome1, currentDir); //busca primeiro arquivo
            
            if( foundIndex1 == -1 && foundFileIndex1 == -1){
                printf("Diretório/arquivo não encontrado.\n");
                continue;
            }

            // busca segundo diretorio
            int foundIndex2 = findDir(nome2, currentDir);
            int foundFileIndex2 = -1;
            
            if (foundIndex2 == -1)
                foundFileIndex2 = findFile(nome2, currentDir); //busca segundo arquivo

            if(foundIndex2 == -1 && foundFileIndex2 == -1) //renomear arquivo/diretorio
            {
                if(foundIndex1 > -1)  // renomear diretorio
                    strcpy(currentDir->subdirs[foundIndex1]->name, nome2);

                if (foundFileIndex1 > -1) // renomear arquivo
                    strcpy(currentDir->files[foundFileIndex1]->name, nome2);
                    
            }

            if (foundIndex1 > -1 && foundIndex2 > -1) //mover pasta pra dentro de pasta
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

            if(foundFileIndex1 > -1 && foundIndex2 > -1) //mover arquivo pra dentro de pasta
            {   
                Dir * dirToReceive = currentDir->subdirs[foundIndex2];
                File *fileToMove = currentDir->files[foundFileIndex1];

                if (dirToReceive->nextDirIndex >= MAX_SUBDIRS)
                {
                    printf("Diretório cheio.\n");
                    continue;
                }

                fileToMove->id = dirToReceive->nextFileid;
                dirToReceive->files[dirToReceive->nextFileIndex] = fileToMove;
                currentDir->files[foundFileIndex1] = NULL;

                dirToReceive->nextFileIndex++;
                dirToReceive->nextFileid++;
                currentDir->nextFileIndex--;

            }

            if(foundFileIndex1 > -1 && foundFileIndex2 > -1){// substituir um arquivo pelo outro
                File *file1 = currentDir->files[foundFileIndex1];

                free(currentDir->files[foundFileIndex2]);

                strcpy(file1->name, nome2);
                currentDir->files[foundFileIndex2] = file1; // file2 passa a apontar pro mesmo endereco que file1
                currentDir->files[foundFileIndex1] = NULL;

                if (foundFileIndex1 < MAX_FILES - 1)
                {
                    // todos os indices depois do file encontrado tem que andar 1 pra tras.
                    for (int i = foundFileIndex1 + 1; i < MAX_FILES; i++)
                    {
                        if (currentDir->files[i] == NULL)
                            break;

                        currentDir->files[i - 1] = currentDir->files[i];
                        currentDir->files[i] = NULL;
                    }
                }

                currentDir->nextFileIndex--;
            }

            if (foundIndex1 > -1 && foundFileIndex2 > -1) // mv (pasta) (arquivo existente) -> erro
            {
                printf("Not a directory.\n");
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
        else if (strcmp(comando, "cp") == 0 && nome1 != NULL && nome2 != NULL)
        {
            if (currentDir->nextFileIndex >= MAX_FILES) // olha apenas no diretorio atual - ToDo: considerar repositorios aninhados 
            {
                printf("Diretório cheio. ");
                printf("Limite de %d arquivos atingido.\n", MAX_FILES);
                continue;
            }

            int foundIndex = findFile(nome1, currentDir);
            if (foundIndex == -1){
                printf("Arquivo não encontrado.\n");
                continue;
            }
            
            int foundIndex2 = findFile(nome2, currentDir);
            int foundDirIndex = findDir(nome2, currentDir);

            if (foundIndex2 == -1 && foundDirIndex == -1) // cp (arquivo) (arquivo n existente) -> criar copia do arquivo com nome do arquivo2
            { 
                if(currentDir->nextFileIndex >= MAX_FILES){
                    printf("Diretório cheio!\n");
                    continue;
                }

                //cria arquivo cópia
                File *copyFile = malloc((sizeof(File)));
                *copyFile = *currentDir->files[foundIndex];
                strcpy(copyFile->name, nome2);
                copyFile->id = currentDir->nextFileid++;

                //adiciona arquivo cópia no diretorio atual
                currentDir->files[currentDir->nextFileIndex++] = copyFile;
            }

            if (foundIndex2 == 1) // cp (arquivo) (arquivo existente) -> arquivo 2 vira copia do arquivo1, nomes se mantem
            {
                free(currentDir->files[foundIndex2]);
                
                File *copyFile = malloc((sizeof(File)));
                *copyFile = *currentDir->files[foundIndex];
                strcpy(copyFile->name, nome2);
                copyFile->id = currentDir->nextFileid++;

                // adiciona arquivo cópia no diretorio atual e no mesmo index do arquivo2 original
                currentDir->files[foundIndex2] = copyFile;
            }

            if (foundDirIndex > -1) // cp (arquivo) (pasta) -> copia arquivo pra dentro da pasta
            {
                Dir *destDir = currentDir->subdirs[foundDirIndex];

                if(destDir->nextFileIndex >= MAX_FILES){
                    printf("Diretório cheio!");
                    continue;
                }

                File *copyFile = malloc((sizeof(File)));
                *copyFile = *currentDir->files[foundIndex];
                copyFile->id = destDir->nextFileid++;

                currentDir->subdirs[foundDirIndex]->files[destDir->nextFileIndex++] = copyFile;
            }

            continue;
        }
        // COMANDOS ARQUIVOS
        else if (strcmp(comando, "touch") == 0 && nome1 != NULL){
            
            int foundIndex = findFile(nome1, currentDir);

            if(foundIndex > -1){
                printf("Arquivo com mesmo nome já existe\n");
                continue;
            }

            File * file = createFile(nome1, currentDir);
            addFileToDir(file, currentDir);
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

            currentDir->nextFileIndex--;
            continue;
        }
        else if (strcmp(comando, "cat") == 0 && nome1 != NULL){
            // ler arquivo
            int foundIndex = findFile(nome1, currentDir);
            
            if(foundIndex == -1){
                printf("Arquivo não encontrado\n");
                continue;
            }

            File * file = currentDir->files[foundIndex];

            printf("%s\n", file->content);

            file->accessed = time(NULL);

            continue;
        }
        else if (strcmp(comando, "echo") == 0 && nome1 != NULL && nome2 != NULL && nome3 != NULL)
        {
            
            if (strcmp(nome2, ">") != 0)
            {
                printf("Utilize o padrao: echo content > arquivo.txt\n");
                continue;
            }


            int foundFileIndex = findFile(nome3, currentDir);
            int foundDirIndex = findDir(nome3, currentDir);

            if(foundFileIndex == -1 && foundDirIndex > -1){
                printf("Erro: %s é um diretorio\n", nome3);
                continue;
            }

            if (foundFileIndex > -1) // echo "file content" > (arquivo) -> sobrescreve conteudo do arquivo pelo conteudo fornecido
            {
                strcpy(currentDir->files[foundFileIndex]->content, nome1);
                currentDir->files[foundFileIndex]->modified = time(NULL);
            }

            if (foundFileIndex == -1 && foundDirIndex == -1) // cria arquivo com conteudo fornecido e nome passado
            {
                File *file = createFile(nome3, currentDir);
                strcpy(file->content, nome1);
                addFileToDir(file, currentDir);
            }

            continue;
        }
        else if (strcmp(comando, "stat") == 0 && nome1 != NULL)
        {
            int foundIndex = findFile(nome1, currentDir);

            if(foundIndex == -1){
                printf("Arquivo nao encontrado\n");
                continue;
            }   

            File * file = currentDir->files[foundIndex];

            printf("Name: %s\n", file->name);
            printf("Created: %s", ctime(&file->created));
            printf("Modified: %s", ctime(&file->modified));
            printf("Accessed: %s", ctime(&file->accessed));

            continue;
        }

    return 0;
    }
}
}
