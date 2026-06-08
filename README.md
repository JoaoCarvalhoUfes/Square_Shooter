# Square_Shooter

#### Descrição
Square Shooter é um jogo shooter 2D top-down multiplayer desenvolvido para o trabalho de redes de computadores 2026/1 da Universidade Federal do Espirito Santo. Nele você encontrará 3 armas diferentes que podem ser selecionadas com as teclas `1`, `2` e `3` do teclado, e utilizar o botão esquerdo do mouse para disparar.

Os principais desafios encontrados durante o desenvolvimento foram:

> 1) O desenvolvimento de uma interface gráfica em C utilizando uma biblioteca externa.

Apesar da biblioteca externa raylib facilitar o trabalho, a linguagem C é uma linguagem de mais baixo nível se comparada a outras linguagens (tais como Python, Java, Go...). Dessa forma, foi necessário adaptar-se ao fluxo de trabalho que essa biblioteca define e às suas funções. 

> 2) Utilização de sockets da linguagem C.

Devido ao baixo nível de abstração da linguagem C, é necessário ter controle da quantidade de bytes enviados através do socket para que tanto o cliente, quanto o servidor, possam gerir corretamente a leitura das informações enviadas através do socket. 
Portanto, foi necessário criar um protocolo de comunicação (o qual pode ser consultado no arquivo `shared/packets.h`).

Toda a comunicação entre cliente e servidor utilizam os pacotes/structs definidos (as) no arquivo supracitado.

> 3) Garantia de sincronização entre os jogadores (real-time).

Devido à latência (lag) natural em jogos *real-time*, foi adotado o modelo de *Authoritative Server*, onde o Servidor centraliza as validações (dano, colisão) e propaga o estado global do jogo aos clientes por meio de *Snapshots*. 

Para garantir uma gameplay fluida e sem "teleportes" (*stuttering*), implementamos duas técnicas no Cliente:
- **Interpolação:** Suavização visual do movimento dos adversários na transição de coordenadas entre um pacote e outro.
- **Previsão Local:** O cliente processa seu próprio movimento instantaneamente, sem esperar validação. O Servidor só força uma correção (*rollback*) se a discrepância de pixels entre as telas ultrapassar uma margem de tolerância.

#### Tecnologias utilizadas

O jogo foi desenvolvido em C. E, a principal ferramenta utilizada para desenvolver a interface do jogo é a biblioteca de desenvolvimento de jogos `raylib.h`. Saiba mais sobre a biblioteca em [`raylib.com`](https://www.raylib.com/)!


#### Instalação

O repositório já contém os executáveis prontos para uso. Caso eventualmente seja necessário re-compilar o código fonte (por motivos de compatibilidade), é necessário instalar a biblioteca [`raylib.h`](https://github.com/raysan5/raylib).

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

