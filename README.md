# Trabalho_Redes

#### Descrição
O trabalho é um jogo shooter 2D top-down multiplayer. Nele você encontrará 3 armas diferentes que podem ser selecionadas com as teclas `1`, `2` e `3` do teclado, e utilizar o botão esquerdo do mouse para disparar.

#### Como jogar
Para iniciar uma partida, um dos jogadores deve ser o Host (Servidor). Para isso, basta baixar o arquivo `server` localizado na pasta `executables/` e rodá-lo no terminal colocando como argumento a porta que será usada, da seguinte forma:

```bash
./server 8080 
```

Os demais jogadores devem baixar o arquivo `client` na pasta `executables/` e abri-lo no terminal colocando como argumentos: primeiro o IP da máquina que está hospedando o servidor, e depois a porta que ela está utilizando, da seguinte forma:

```bash
./client 192.168.1.50 8080
```

> **Nota:** Se os arquivos recém-baixados não abrirem por falta de permissão no Linux, basta rodar o comando `chmod +x server` e `chmod +x client` para liberar a execução deles.
