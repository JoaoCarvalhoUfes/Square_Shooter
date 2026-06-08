# Trabalho_Redes

#### Descrição
O trabalho é um jogo shooter 2D top-down multiplayer. Nele você encontrará 3 armas diferentes que podem ser selecionadas com as teclas `1`, `2` e `3` do teclado, e utilizar o botão esquerdo do mouse para disparar.

Os principais desafios encontrados durante o desenvolvimento foram:

> 1) O desenvolvimento de uma interface gráfica em C utilizando uma biblioteca externa.

Apesar da biblioteca externa facilitar o trabalho, a linguagem C é uma linguagem de mais baixo nível se
comparada a outras linguagens (tais como Python, Java, Go...). Portanto, 

> 2) Utilização de sockets da linguagem C.

Devido ao baixo nível de abstração da linguagem C, é necessário ter controle da quantidade de bytes enviados através do socket para que tanto o cliente, quanto o servidor, possam gerir corretamente a leitura das informações enviadas através do socket. 
Portanto, foi necessário criar um protocolo de comunicação (o qual pode ser consultado no arquivo `shared/packets.h`).

Toda a comunicação entre cliente e servidor utilizam os pacotes/structs definidos (as) no arquivo supracitado.

> 3) Garantia de sincronização entre os jogadores (real-time).

(completar)

#### Tecnologias utilizadas

O jogo foi desenvolvido em C. E, a principal ferramenta utilizada para desenvolver a interface do jogo é a biblioteca de desenvolvimento de jogos `raylib.h`. Saiba mais sobre a biblioteca em [`raylib.com`](https://www.raylib.com/)!


#### Instalação

O repositório já contém os executáveis compatíveis com a distro Ubuntu 24.04.03 LTS. 
Caso eventualmente seja necessário re-compilar o código fonte (por motivos de compatibilidade), é necessário instalar 
a biblioteca [`raylib.h`](https://github.com/raysan5/raylib).

> **Nota:** Para re-compilar os arquivos de execução, basta rodar o comando `make` nas pastas `client/` e `server/`

#### Como jogar
Para iniciar uma partida, um dos jogadores deve ser o Host (Servidor). Para isso, basta baixar o arquivo `server` localizado na pasta `executables/` e rodá-lo no terminal colocando como argumento a porta que será usada, da seguinte forma:

```bash
./server 8080 
```

Os demais jogadores devem baixar o arquivo `client` na pasta `executables/` e abri-lo no terminal colocando como argumentos: primeiro o IP da máquina que está hospedando o servidor, e depois a porta que ela está utilizando, da seguinte forma:

```bash
./client 192.168.1.50 8080
```

> **Nota 1:** Se os arquivos recém-baixados não abrirem por falta de permissão no Linux, basta rodar o comando `chmod +x server` e `chmod +x client` para liberar a execução deles.

