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

    todo arquivo e diretorio pertence a um
        (owner), grupo (group) e outros (public)

        read, write and execute

        r (4), w (2) e x (1)

        UGO (User, Group, Others)

        chmod u+w arquivo -> adiciona permissao de escrita pro user (u) no arquivo
        

        chmod 755 pasta ou chmod 644 arquivo.txt


        chown: altera o usuário dono e/ou o grupo do item (ex.: sudo chown usuario:grupo arquivo)


    3.4. Simulação de Alocação de Blocos
*/

#define MAX_SUBDIRS 5
#define MAX_FILES 50
#define MAX_BLOCKS_PER_FILE 50
#define TOTAL_BLOCKS 4
#define BLOCK_SIZE 4

char blocks[TOTAL_BLOCKS][BLOCK_SIZE];
int blockUsed[TOTAL_BLOCKS] = {0};
#define CURRENT_USER 0 // 0 user, 1 group, 2 other

typedef struct File
{
    int id;
    char name[20];
    //int tamanho; // bytes
    char content[200];
    int size;
    int blockCount;
    int blockIndices[MAX_BLOCKS_PER_FILE]; // alocação com indexação simples

    time_t created;
    time_t modified;
    time_t accessed;

    int permissions[3]; // r, w, x -> chmod numero
    /*
        r = 4 (read)
        w = 2 (write)
        x = 1 (execute)
         
        permissions = [4, 0, 0]
    */

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

    int permissions[3]; // r, w, x -> chmod numero
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
    if (file == NULL)
        return NULL;
    strcpy(file->name, name);
    file->content[0] = '\0';
    file->size = 0;
    file->blockCount = 0;
    file->created = time(NULL);
    file->modified = time(NULL);
    file->accessed = time(NULL);
    file->id = dir->nextFileid++;

    file->permissions[0] = 0; // r
    file->permissions[1] = 0; // w
    file->permissions[2] = 0; // x

    return file;
}

Dir *createDir(char name[10], Dir *dir) {
    Dir *newDir = (Dir *)malloc(sizeof(Dir)); // alocação dinâmica de memória
    strcpy(newDir->name, name);
    newDir->nextDirIndex = 0;
    newDir->parent = dir;
    for (int i = 0; i < MAX_SUBDIRS; i++)
    {
        newDir->subdirs[i] = NULL;
    }
    for (int i = 0; i < MAX_FILES; i++)
    {
        newDir->files[i] = NULL;
    }

    if(dir == NULL){ // root
        newDir->permissions[0] = 7; // r
        newDir->permissions[1] = 7; // w
        newDir->permissions[2] = 7; // x
    }else{
        newDir->permissions[0] = 0; // r
        newDir->permissions[1] = 0; // w
        newDir->permissions[2] = 0; // x
    }
 

    return newDir;
}

int hasReadPermission(int permission){
    if (permission == 4 || permission == 5 || permission == 6 || permission == 7)
        return 1;

    return 0;
}

int hasWritePermission(int permission)
{
    if (permission == 2 || permission == 6 || permission == 3 || permission == 7)
        return 1;

    return 0;
}

int hasExecPermission(int permission)
{
    if (permission == 1 || permission == 3 || permission == 5 || permission == 7)
        return 1;

    return 0;
}

void addFileToDir(File * file, Dir * dir){
    dir->files[dir->nextFileIndex] = file;
    dir->nextFileIndex++;
}

int main() {
    Dir *currentDir = createDir("root", NULL);
    char linha[200];

    while(1){
        printf("/%s:", currentDir->name);
        // Lê as duas palavras separadas por espaço
        if (fgets(linha, sizeof linha, stdin) != NULL) {

             char *comando = strtok(linha, " \t\n");
             char *nome1 = strtok(NULL, " \t\n");
             char *nome2 = strtok(NULL, " \t\n");
             char *nome3 = strtok(NULL, " \t\n");

        if (comando == NULL)
        {
                 continue;
                 // COMANDOS DIRETORIOS
        } else if (strcmp(comando, "rmdir") == 0 && nome1 != NULL)
        {
            int permission = currentDir->permissions[CURRENT_USER];

            if (!hasWritePermission(permission))
            {
                printf("User %d: Permissao negada.\n", CURRENT_USER);
                continue;
            }

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
            int permission = currentDir->permissions[CURRENT_USER];

            if (!hasWritePermission(permission))
            {
                printf("User %d: Permissao negada.\n", CURRENT_USER);
                continue;
            }

            if (currentDir->nextDirIndex >= MAX_SUBDIRS)
            {
                printf("Diretório cheio. ");
                printf("Limite de %d pastas atingido.\n", MAX_SUBDIRS);
                continue;
            }

            Dir * newDir = createDir(nome1, currentDir);

            currentDir->subdirs[currentDir->nextDirIndex] = newDir; // Adiciona o novo diretório à raiz
            currentDir->nextDirIndex++;
            continue;
        }

         else if (strcmp(comando, "ls") == 0)
        {
            int permission = currentDir->permissions[CURRENT_USER];

            if(!hasReadPermission(permission)){
                printf("User %d: Permissao negada.\n", CURRENT_USER);
                continue;
            }

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

                Dir * dir = currentDir->subdirs[foundIndex];
                int permission = dir->permissions[CURRENT_USER];

                if (!hasExecPermission(permission))
                {
                    printf("User %d: Permissao negada.\n", CURRENT_USER);
                    continue;
                }

                currentDir = currentDir->subdirs[foundIndex];      
            }

            continue;
        }
        // ======================
        else if (strcmp(comando, "cp") == 0 && nome1 != NULL && nome2 != NULL)
        {
            int foundIndex = findFile(nome1, currentDir);
            if (foundIndex == -1)
            {
                printf("Arquivo não encontrado.\n");
                continue;
            }

            File *file1 = currentDir->files[foundIndex];
            int permission = file1->permissions[CURRENT_USER];
            if (!hasReadPermission(permission))
            {
                printf("User %d: Permissao negada.\n", CURRENT_USER);
                continue;
            }

            int foundIndex2 = findFile(nome2, currentDir);
            int foundDirIndex = findDir(nome2, currentDir);

            Dir *destDir = currentDir;
            const char *copyName = nome2;
            int replaceIndex = -1;

            if (foundDirIndex >= 0)
            {
                destDir = currentDir->subdirs[foundDirIndex];
                copyName = file1->name;
                if (findFile((char *)copyName, destDir) >= 0)
                {
                    printf("Já existe um arquivo com esse nome no diretório de destino.\n");
                    continue;
                }
            }
            else if (foundIndex2 >= 0)
            {
                if (foundIndex2 == foundIndex)
                {
                    printf("Origem e destino são o mesmo arquivo.\n");
                    continue;
                }

                File *destinationFile = currentDir->files[foundIndex2];
                if (!hasWritePermission(destinationFile->permissions[CURRENT_USER]))
                {
                    printf("Sem permissão para substituir o arquivo de destino.\n");
                    continue;
                }
                replaceIndex = foundIndex2;
            }

            if (!hasWritePermission(destDir->permissions[CURRENT_USER]))
            {
                printf("Sem permissão de escrita no diretório de destino.\n");
                continue;
            }

            if (strlen(copyName) >= sizeof(file1->name))
            {
                printf("Nome do arquivo muito longo.\n");
                continue;
            }

            if (replaceIndex == -1 && destDir->nextFileIndex >= MAX_FILES)
            {
                printf("Diretório cheio.\n");
                continue;
            }

            int availableBlocks = 0;
            for (int i = 0; i < TOTAL_BLOCKS; i++)
            {
                if (!blockUsed[i])
                    availableBlocks++;
            }
            if (replaceIndex >= 0)
            {
                File *oldDestination = currentDir->files[replaceIndex];
                availableBlocks += oldDestination->blockCount;
            }

            if (availableBlocks < file1->blockCount)
            {
                printf("Espaço insuficiente nos blocos para uma cópia independente.\n");
                continue;
            }

            File *copyFile = malloc(sizeof *copyFile);
            if (copyFile == NULL)
            {
                printf("Não foi possível alocar memória para a cópia.\n");
                continue;
            }

            *copyFile = *file1;
            strcpy(copyFile->name, copyName);
            copyFile->id = destDir->nextFileid++;
            copyFile->blockCount = 0;

            /* Ao substituir, libera os blocos do destino somente após validar a cópia. */
            if (replaceIndex >= 0)
            {
                File *oldDestination = currentDir->files[replaceIndex];
                for (int i = 0; i < oldDestination->blockCount; i++)
                    blockUsed[oldDestination->blockIndices[i]] = 0;
                free(oldDestination);
            }

            /* Reserva blocos diferentes dos usados pelo arquivo original. */
            for (int i = 0; i < TOTAL_BLOCKS && copyFile->blockCount < file1->blockCount; i++)
            {
                if (!blockUsed[i])
                {
                    int sourceBlock = file1->blockIndices[copyFile->blockCount];
                    memcpy(blocks[i], blocks[sourceBlock], BLOCK_SIZE);
                    blockUsed[i] = 1;
                    copyFile->blockIndices[copyFile->blockCount] = i;
                    copyFile->blockCount++;
                }
            }

            if (replaceIndex >= 0)
            {
                currentDir->files[replaceIndex] = copyFile;
            }
            else
            {
                destDir->files[destDir->nextFileIndex++] = copyFile;
            }

            continue;
        }
        // COMANDOS ARQUIVOS
        else if (strcmp(comando, "touch") == 0 && nome1 != NULL){
            
            int permission = currentDir->permissions[CURRENT_USER];

            if (!hasWritePermission(permission))
            {
                printf("User %d: Permissao negada.\n", CURRENT_USER);
                continue;
            }

            if (currentDir->nextFileIndex >= MAX_FILES) {
                printf("Diretório cheio.\n");
                continue;
            }

            if (strlen(nome1) >= sizeof(((File *)0)->name))
            {
                printf("Nome do arquivo muito longo.\n");
                continue;
            }

            int foundIndex = findFile(nome1, currentDir);

            if(foundIndex > -1){
                printf("Arquivo com mesmo nome já existe\n");
                continue;
            }

            File * file = createFile(nome1, currentDir);
            if (file == NULL) {
                printf("Não foi possível criar o arquivo.\n");
                continue;
            }
            addFileToDir(file, currentDir);
            continue;
        }

        else if (strcmp(comando, "rm") == 0 && nome1 != NULL)
        {
            int permission = currentDir->permissions[CURRENT_USER];

            if (!hasWritePermission(permission))
            {
                printf("User %d: Permissao negada.\n", CURRENT_USER);
                continue;
            }

            // procurar arquivo
            int foundIndex = findFile(nome1, currentDir);
            
            if (foundIndex == -1)
            {
                printf("Arquivo não encontrado.");
                continue;
            }

            File *file = currentDir->files[foundIndex];

            for (int i = 0; i < file->blockCount; i++) {
                blockUsed[file->blockIndices[i]] = 0;
            }

            free(file);

            // desloca as entradas do array para preencher a posição removida
            for (int i = foundIndex; i < currentDir->nextFileIndex - 1; i++) {
                currentDir->files[i] = currentDir->files[i + 1];
            }

            currentDir->files[currentDir->nextFileIndex - 1] = NULL;
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

            int permission = file->permissions[CURRENT_USER];

            if (hasReadPermission(permission) == 0)
            {
                printf("User %d: Permissao negada.\n", CURRENT_USER);
                continue;
            }

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

            if (strlen(nome3) >= sizeof(((File *)0)->name))
            {
                printf("Conteudo do arquivo muito longo.\n");
                continue;
            }

            int foundFileIndex = findFile(nome3, currentDir);
            int foundDirIndex = findDir(nome3, currentDir);

            if (foundFileIndex == -1 && foundDirIndex > -1)
            {
                printf("Erro: %s é um diretorio\n", nome3);
                continue;
            }

            File *file = NULL;
            

                if (foundFileIndex > -1) // echo "file content" > (arquivo) -> sobrescreve conteudo do arquivo pelo conteudo fornecido
                {
                    file = currentDir->files[foundFileIndex];
                    int permission = file->permissions[CURRENT_USER];

                    if (!hasWritePermission(permission))
                    {
                        printf("User %d: Permissao negada.\n", CURRENT_USER);
                        continue;
                    }

                    strcpy(currentDir->files[foundFileIndex]->content, nome1);
                    currentDir->files[foundFileIndex]->modified = time(NULL);
                }

                /* Os blocos atuais do arquivo poderão ser reutilizados. */
                int available = file != NULL ? file->blockCount : 0;
                for (int i = 0; i < TOTAL_BLOCKS; i++)
                {
                    if (blockUsed[i] == 0)
                        available++;
                }

                int size = (int)strlen(nome1);
           
                int needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

                if (needed > MAX_BLOCKS_PER_FILE ||
                    size >= sizeof(((File *)0)->content))
                {
                    printf("Conteúdo grande demais.\n");
                    continue;
                }

                if (available < needed)
                {
                    printf("Espaço insuficiente nos blocos.\n");
                    continue;
                }

                /* Se for arquivo novo, verifica capacidade antes de criá-lo. */
                if (file == NULL)
                {
                    int permission = file->permissions[CURRENT_USER];

                    if (!hasWritePermission(permission))
                    {
                        printf("User %d: Permissao negada.\n", CURRENT_USER);
                        continue;
                    }

                    if (currentDir->nextFileIndex >= MAX_FILES)
                    {
                        printf("Diretório cheio.\n");
                        continue;
                    }

                    file = createFile(nome3, currentDir);
                    if (file == NULL)
                    {
                        printf("Não foi possível criar o arquivo.\n");
                        continue;
                    }
                    addFileToDir(file, currentDir);
                }

                /* Libera os blocos antigos para reutilizá-los na nova gravação. */
                for (int i = 0; i < file->blockCount; i++)
                {
                    blockUsed[file->blockIndices[i]] = 0;
                }
                file->blockCount = 0;

                /* Reserva blocos e grava neles partes do conteúdo. */
                for (int i = 0; i < TOTAL_BLOCKS && file->blockCount < needed; i++)
                {
                    if (blockUsed[i] == 0)
                    {
                        int offset = file->blockCount * BLOCK_SIZE;
                        int bytes = size - offset;

                        if (bytes > BLOCK_SIZE)
                            bytes = BLOCK_SIZE;

                        memcpy(blocks[i], nome1 + offset, bytes);
                        blockUsed[i] = 1;
                        file->blockIndices[file->blockCount] = i;
                        file->blockCount++;
                    }
                }

                /* Atualiza os dados do arquivo após concluir a alocação. */
                strcpy(file->content, nome1);
                file->size = size;
                file->modified = time(NULL);

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
            
            printf("Permissions: ");
            printf("%d", file->permissions[0]);
            printf("%d", file->permissions[1]);
            printf("%d", file->permissions[2]);
            printf("\n");

            continue;
        }
        else if (strcmp(comando, "chmod") == 0 && nome1 != NULL && nome2 != NULL){
            if (strlen(nome1) != 3)
            {
                printf("Comando inválido\n");
                continue;
            }

            int foundFileIndex = findFile(nome2, currentDir);
            int foundDirIndex = findDir(nome2, currentDir);

            if (foundFileIndex == -1 && foundDirIndex == -1)
            {
                printf("Arquivo/diretorio não encontrado\n");
                continue;
            }

            if(foundFileIndex > -1){
                currentDir->files[foundFileIndex]->permissions[0] = nome1[0] - '0';
                currentDir->files[foundFileIndex]->permissions[1] = nome1[1] - '0';
                currentDir->files[foundFileIndex]->permissions[2] = nome1[2] - '0';
                continue;
            }
            
            if(foundDirIndex > -1){
                currentDir->subdirs[foundDirIndex]->permissions[0] = nome1[0] - '0';
                currentDir->subdirs[foundDirIndex]->permissions[1] = nome1[1] - '0';
                currentDir->subdirs[foundDirIndex]->permissions[2] = nome1[2] - '0';
                continue;
            }
 

            continue;
        }

            return 0;
    }
}
}
