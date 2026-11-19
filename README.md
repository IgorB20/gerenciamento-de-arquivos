# Simulador de gerenciamento de arquivos em C

Trabalho M3 de Sistemas Operacionais. O simulador oferece um terminal para criar, organizar e manipular arquivos e diretórios de um sistema de arquivos em memória. A sessão começa no diretório `/root` simulado. Os comandos digitados nesse terminal operam sobre as estruturas do programa, e os dados da sessão são descartados ao encerrar o processo.

## Compilação e execução

Use um compilador C como GCC ou Clang e rode os comandos abaixo na raiz do repositório.

```sh
cc -std=c11 main.c -o simulador.out
./simulador.out
```

O executável é compilado a partir de [`main.c`](main.c). [`ponteiros.c`](ponteiros.c) é um exemplo separado com sua própria função `main`. Quando aparecer o prompt `/root:`, digite um comando por linha. Use **Ctrl+C** para encerrar a sessão.

## Arquivos, FCB e inode simulado

Um arquivo reúne dados e metadados. Neste projeto, a estrutura `File` funciona como o **File Control Block (FCB)** de cada arquivo. O programa aloca cada FCB dinamicamente com `malloc`, e o diretório guarda um ponteiro `File *` para ele.

| Campo de `File` | Papel no simulador |
| --- | --- |
| `name` | Nome usado para localizar o arquivo dentro de um diretório. |
| `size` | Tamanho do conteúdo em bytes, atualizado após uma gravação bem-sucedida. |
| `created`, `modified`, `accessed` | Datas de criação, modificação e último acesso, representadas com `time_t`. `cat` atualiza a data de acesso e `echo` atualiza a de modificação. |
| `permissions[3]` | Máscaras de leitura, escrita e execução para proprietário, grupo e outros. |
| `id` | Identificador numérico exibido por `ls` ao lado do nome. |
| `blockCount`, `blockIndices[]` | Quantidade de blocos e sequência de índices dos blocos de dados. |
| `content` | Cópia do conteúdo em uma string, usada diretamente por `cat`. |

O campo `id` faz o papel de **inode simulado**, ilustrando a diferença entre o nome e a identidade numérica. No Linux, o número do inode identifica um objeto dentro do sistema de arquivos, e a entrada de diretório associa um nome a esse inode. Aqui, o nome está no próprio FCB e o código usa o contador `nextFileid` do diretório para atribuir IDs. Renomear mantém o ID, enquanto mover um arquivo para um subdiretório atribui um ID do diretório de destino. O simulador trata o conteúdo como texto e o FCB ainda carece de um campo de tipo de arquivo.

## Árvore de diretórios

Cada nó da árvore é uma estrutura `Dir`. Ela armazena o nome, as permissões e dois vetores de ponteiros. `subdirs[]` aponta para os diretórios filhos e `files[]` aponta para os FCBs dos arquivos. O ponteiro `parent` permite subir um nível com `cd ..`, e tem valor `NULL` na raiz. `currentDir` mantém a referência ao diretório em que os comandos são executados.

- **Eficiência.** Para localizar um nome, `findDir` e `findFile` percorrem somente os vetores do diretório relevante. A busca é linear nas entradas desse diretório. Subir um nível exige apenas seguir `parent`.
- **Nomeação.** Cada diretório fornece o contexto para interpretar um nome. Dois diretórios diferentes podem ter um arquivo chamado `nota`.
- **Agrupamento.** Diretórios podem conter arquivos e outros diretórios, o que permite organizar os dados por assunto. Para chegar a uma pasta aninhada, execute um `cd` por nível.

Os vetores têm capacidade de cinco subdiretórios e cinquenta arquivos por diretório. O prompt mostra o nome do diretório atual. Os comandos resolvem nomes locais, e caminhos como `aulas/exerc` ficam fora da sintaxe suportada.

## Proteção de acesso e `chmod`

O vetor `permissions[3]` guarda uma máscara por classe, na ordem **proprietário, grupo, outros**. Cada permissão ocupa um bit do dígito octal.

| Permissão | Valor | Arquivo | Diretório |
| --- | --- | --- | --- |
| `r` (leitura) | `4` | Ler o conteúdo | Listar as entradas |
| `w` (escrita) | `2` | Alterar o conteúdo | Criar ou remover entradas |
| `x` (execução) | `1` | Executar, como conceito | Entrar com `cd` |

`chmod 640 nota` converte os três caracteres em inteiros e grava `[6, 4, 0]`. O proprietário recebe leitura e escrita (`4 + 2`), o grupo recebe leitura e os outros ficam com máscara zero. As funções `hasReadPermission`, `hasWritePermission` e `hasExecPermission` verificam o bit correspondente por meio dos valores que o contêm. Por exemplo, para leitura elas aceitam `4`, `5`, `6` ou `7`, o que equivale a testar `(permissao & 4) != 0` para máscaras válidas.

`CURRENT_USER` seleciona a classe consultada e vale `0` (proprietário) na configuração atual. Grupo e outros estão representados nas máscaras, e o programa usa uma classe fixa durante a sessão. A raiz começa com modo `777`. Arquivos e subdiretórios novos começam com `000`. Para usar `cat` e `echo` em um arquivo novo, dê ao proprietário leitura e escrita com `chmod 600`. Para entrar, listar e criar entradas em um subdiretório novo, use `chmod 700` antes de entrar.

O código verifica leitura do arquivo em `cat` e na origem de `cp`, escrita do arquivo em `echo` sobre um arquivo existente, leitura do diretório em `ls`, escrita do diretório em `touch`, `mkdir`, `rm` e `rmdir`, e execução no diretório de destino ao entrar com `cd`. O bit de execução em arquivos é armazenado para demonstrar a máscara. A interface carece de um comando para executar arquivos.

## Simulação de alocação de blocos

O disco simulado é a matriz global `blocks[4][4]`, com **quatro blocos de quatro bytes** e capacidade total de **16 bytes de conteúdo**, compartilhada entre os arquivos. O vetor `blockUsed[]` registra quais blocos estão ocupados. Em uma gravação bem-sucedida, `echo` calcula quantos blocos a string exige arredondando o tamanho dividido por quatro para cima. Em seguida, libera os blocos anteriores do arquivo, percorre os blocos livres a partir do índice zero e copia até quatro bytes para cada bloco reservado. Um texto de cinco bytes, por exemplo, ocupa dois blocos.

| Estratégia | Organização dos dados | Relação com o projeto |
| --- | --- | --- |
| Contínua | O arquivo ocupa uma sequência consecutiva. A posição inicial e o comprimento bastam para localizar os dados, mas expandir o arquivo pode exigir mover a sequência. | Referência teórica. |
| Encadeada | Cada bloco indica o próximo. Os blocos podem estar espalhados, e chegar a um bloco intermediário exige seguir a cadeia. | Referência teórica. |
| Indexada | Uma tabela registra em ordem os blocos de um arquivo. O índice lógico dá acesso direto ao bloco físico correspondente, mesmo que os blocos estejam separados. | Implementada com `File.blockIndices[]` no FCB. |

Na escrita, `blockIndices[0]` indica onde estão os primeiros quatro bytes, `blockIndices[1]` indica o próximo trecho, e assim por diante. Como o vetor aceita índices separados, uma gravação pode usar blocos livres intercalados com blocos de outros arquivos. A leitura por `cat` usa `File.content`. Assim, a escrita demonstra a alocação, enquanto a leitura ainda carece da reconstrução do conteúdo a partir dos blocos.

## Comandos e correspondência com o Linux

| No simulador | Operação | Diferença para o Linux |
| --- | --- | --- |
| `mkdir aulas` | Cria um subdiretório. | No Linux, o modo inicial normalmente depende da `umask`. Aqui, começa com `000`. |
| `ls` | Lista subdiretórios e arquivos do diretório atual. | O simulador mostra os IDs dos arquivos na listagem padrão. No Linux, `ls -i` mostra os números de inode. |
| `cd aulas` / `cd ..` | Entra em um subdiretório ou sobe ao pai. | O shell do Linux também resolve caminhos absolutos e com vários níveis. |
| `touch nota` | Cria um arquivo vazio. | No Linux, `touch` em um arquivo existente atualiza datas. O simulador informa que o nome já existe. |
| `echo ABCD > nota` | Sobrescreve o conteúdo de um arquivo com permissão de escrita. | No Linux, o shell interpreta `>` e pode criar o destino. Aqui, o próprio simulador interpreta `>` e espera uma única palavra de conteúdo. Use um arquivo criado com `touch`. |
| `cat nota` | Mostra o conteúdo e atualiza a data de acesso. | A leitura do Linux passa pelo sistema de arquivos. O simulador imprime a string do FCB e acrescenta uma quebra de linha na exibição. |
| `cp nota copia` | Cria uma cópia com outro nome, ou aceita um subdiretório como destino. | O Linux oferece cópia recursiva com `cp -r`. O simulador aceita arquivos como origem. |
| `mv copia resumo` | Renomeia um arquivo ou diretório. Também há lógica para mover itens para subdiretórios. | O Linux aceita caminhos como origem e destino. O simulador busca os dois nomes no diretório atual. |
| `rm resumo` | Remove a entrada do arquivo e libera o FCB em memória. | No Linux, remover o último link de um arquivo fechado também permite recuperar o espaço de seus dados. A versão atual do simulador deixa os blocos marcados como ocupados. |
| `rmdir aulas` | Remove um subdiretório do diretório atual. | O Linux exige que o diretório esteja vazio. O simulador ainda carece dessa validação e da liberação dos itens internos. |
| `stat nota` | Mostra nome, datas e permissões. | O `stat` do Linux também mostra informações como tamanho, inode e número de blocos. |
| `chmod 600 nota` | Define as três máscaras de permissão. | O Linux também aceita modos simbólicos, como `chmod u+rw nota`. O simulador recebe a forma numérica de três caracteres. |

## Exemplos de uso

### Criar, escrever, ler e organizar

Com o simulador recém-iniciado, digite a sequência abaixo dentro dele.

```text
mkdir aulas
chmod 700 aulas
cd aulas
touch nota
cat nota
chmod 600 nota
echo ABCDEFGH > nota
cat nota
stat nota
cp nota copia
mv copia resumo
ls
rm resumo
rm nota
cd ..
rmdir aulas
```

O primeiro `cat` retorna `User 0: Permissao negada.` porque `nota` começa com modo `000`. O segundo imprime `ABCDEFGH` depois da mudança para `600` e da gravação. `stat` exibe o modo `600` e as datas da sessão. `ls` mostra `nota` e `resumo` com seus IDs.

### Usar blocos livres separados

Reinicie o simulador para começar com os quatro blocos livres.

```text
touch a
touch b
touch c
chmod 600 a
chmod 600 b
chmod 600 c
echo ABCDEFGH > a
echo IJKL > b
echo ABCD > a
echo MNOPQRST > c
cat c
echo MNOPQRSTUVWX > c
```

Inicialmente, `a` usa os blocos 0 e 1, e `b` usa o bloco 2. Ao regravar `a` com quatro bytes, o bloco 1 fica livre. Em seguida, `c` recebe os dois blocos livres, 1 e 3, e `cat c` imprime `MNOPQRST`. A última gravação exigiria três blocos para `c`. Como apenas seus dois blocos atuais estão disponíveis para reaproveitamento, a operação informa `Espaço insuficiente nos blocos.` Os índices desse exemplo seguem a ordem de busca usada em `echo`. A interface ainda carece de um comando para listar os blocos.

## Limitações da versão atual

- `Dir.nextFileIndex` e `Dir.nextFileid` ainda precisam de inicialização explícita na criação de um diretório. As posições de inserção e os IDs podem variar conforme o conteúdo da memória alocada.
- `echo` para um destino inexistente pode encerrar o processo por acessar um ponteiro nulo. Ao sobrescrever, o comando atualiza a string antes de verificar o espaço, de modo que uma gravação recusada pode deixar o conteúdo exibido diferente dos dados alocados.
- `cp` copia os índices de blocos do arquivo original junto com os demais campos do FCB. Origem e cópia ficam com referências aos mesmos blocos, embora cada FCB tenha sua própria string. As rotinas de cópia, remoção e substituição ainda carecem de gerenciamento consistente desses blocos.
- A sobrescrita de um arquivo existente via `cp` só entra no ramo correspondente quando o destino está na posição 1 do vetor. Em `rm`, buscar um nome inexistente pode acessar uma entrada nula. Mover uma pasta para outra com `mv` também deixa o ponteiro `parent` da pasta movida apontando para o pai anterior.
- O controle de acesso é parcial. `mv` e `chmod` carecem de validação da autorização do usuário. `chmod` também checa o comprimento do modo, mas aceita caracteres fora do intervalo de `0` a `7`.