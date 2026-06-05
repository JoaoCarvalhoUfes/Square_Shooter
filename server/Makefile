# Variáveis de ferramentas e flags de compilação (Avisos e Debug)
CC = gcc
CFLAGS = -Wall -Wextra -g

# Bibliotecas de linkagem (Ficam separadas!)
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
TARGET = server

# 1. Alvo principal: Agora com TODAS as dependências listadas corretamente
$(TARGET): main.o player.o packets.o server_controller.o movement_validation.o aim.o weapon.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o player.o packets.o server_controller.o movement_validation.o aim.o weapon.o $(LIBS)

# 2. Compilação dos objetos da raiz
main.o: main.c player.h packets.h ./controller/server_controller.h ../config.h
	$(CC) $(CFLAGS) -c main.c

player.o: player.c player.h ../config.h 
	$(CC) $(CFLAGS) -c player.c

packets.o: packets.c packets.h ../config.h
	$(CC) $(CFLAGS) -c packets.c

# 3. Compilação dos objetos em subpastas (Atenção aos caminhos dos arquivos .c)
server_controller.o: ./controller/server_controller.c ./controller/server_controller.h ./validation/movement_validation.h player.h
	$(CC) $(CFLAGS) -c ./controller/server_controller.c

movement_validation.o: ./validation/movement_validation.c ./validation/movement_validation.h player.h ../config.h
	$(CC) $(CFLAGS) -c ./validation/movement_validation.c

aim.o: ./aim.c ./aim.h ../config.h
	$(CC) $(CFLAGS) -c ./aim.c

weapon.o: ./weapon.c ./weapon.h ../config.h
	$(CC) $(CFLAGS) -c ./weapon.c

# 4. Alvo de limpeza
.PHONY: clean
clean:
	rm -f *.o $(TARGET)