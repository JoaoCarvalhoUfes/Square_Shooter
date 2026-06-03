# Variáveis de ferramentas e flags de compilação (Avisos e Debug)
CC = gcc
CFLAGS = -Wall -Wextra -g

# Bibliotecas de linkagem (Ficam separadas!)
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
TARGET = game

# 1. Alvo principal: Agora com TODAS as dependências listadas corretamente
$(TARGET): main.o player.o packets.o client_controller.o api.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o player.o packets.o client_controller.o api.o $(LIBS)

# 2. Compilação dos objetos da raiz
main.o: main.c player.h packets.h api.h packets.h ./controller/client_controller.h ../config.h
	$(CC) $(CFLAGS) -c main.c

player.o: player.c player.h ../config.h 
	$(CC) $(CFLAGS) -c player.c

packets.o: packets.c packets.h ../config.h
	$(CC) $(CFLAGS) -c packets.c

# 3. Compilação dos objetos em subpastas (Atenção aos caminhos dos arquivos .c)
client_controller.o: ./controller/client_controller.c ./controller/client_controller.h ../config.h
	$(CC) $(CFLAGS) -c ./controller/client_controller.c

api.o: ./api.c ./api.h packets.h ../config.h
	$(CC) $(CFLAGS) -c ./api.c

# 4. Alvo de limpeza
.PHONY: clean
clean:
	rm -f *.o $(TARGET)