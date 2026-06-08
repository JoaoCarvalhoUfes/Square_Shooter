# Variáveis de ferramentas e flags de compilação (Avisos e Debug)
CC = gcc
CFLAGS = -Wall -Wextra -g

# Bibliotecas de linkagem (Ficam separadas!)
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
TARGET = server

# 1. Alvo principal: Agora com TODAS as dependências listadas corretamente
$(TARGET): main.o player.o packets.o server_controller.o utils.o aim.o weapon.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o player.o packets.o server_controller.o utils.o aim.o weapon.o $(LIBS)
	mkdir -p ../executables
	cp $(TARGET) ../executables/

# 2. Compilação dos objetos da raiz
main.o: main.c ../shared/player.h ../shared/packets.h ./controller/server_controller.h ../config.h
	$(CC) $(CFLAGS) -c main.c

player.o: ../shared/player.c ../shared/player.h ../config.h 
	$(CC) $(CFLAGS) -c ../shared/player.c

packets.o: ../shared/packets.c ../shared/packets.h ../config.h
	$(CC) $(CFLAGS) -c ../shared/packets.c

# 3. Compilação dos objetos em subpastas (Atenção aos caminhos dos arquivos .c)
server_controller.o: ./controller/server_controller.c ./controller/server_controller.h ../shared/utils.h ../shared/player.h
	$(CC) $(CFLAGS) -c ./controller/server_controller.c

utils.o: ../shared/utils.c ../shared/utils.h ../shared/player.h ../config.h
	$(CC) $(CFLAGS) -c ../shared/utils.c

aim.o: ../shared/aim.c ../shared/aim.h ../config.h
	$(CC) $(CFLAGS) -c ../shared/aim.c

weapon.o: ../shared/weapon.c ../shared/weapon.h ../config.h
	$(CC) $(CFLAGS) -c ../shared/weapon.c

# 4. Alvo de limpeza
.PHONY: clean
clean:
	rm -f *.o $(TARGET)