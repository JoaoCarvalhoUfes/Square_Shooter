# Variáveis de ferramentas e flags de compilação (Avisos e Debug)
CC = gcc
CFLAGS = -Wall -Wextra -g

# Bibliotecas de linkagem (Ficam separadas!)
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
TARGET = client

# 1. Alvo principal: Agora com TODAS as dependências listadas corretamente
$(TARGET): main.o player.o packets.o client_controller.o api.o aim.o weapon.o utils.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o player.o packets.o client_controller.o api.o aim.o weapon.o utils.o $(LIBS)
	mkdir -p ../executables
	cp $(TARGET) ../executables/

# 2. Compilação dos objetos da raiz

main.o: main.c ../shared/player.h ../shared/packets.h api.h ./controller/client_controller.h ../config.h ../shared/aim.h
	$(CC) $(CFLAGS) -c main.c

player.o: ../shared/player.c ../shared/player.h ../config.h 
	$(CC) $(CFLAGS) -c ../shared/player.c

packets.o: ../shared/packets.c ../shared/packets.h ../config.h
	$(CC) $(CFLAGS) -c ../shared/packets.c

client_controller.o: ./controller/client_controller.c ./controller/client_controller.h ../config.h ../shared/utils.h
	$(CC) $(CFLAGS) -c ./controller/client_controller.c

utils.o: ../shared/utils.c ../shared/utils.h ../shared/player.h ../config.h
	$(CC) $(CFLAGS) -c ../shared/utils.c

api.o: ./api.c ./api.h ../shared/packets.h ../config.h
	$(CC) $(CFLAGS) -c ./api.c

aim.o: ../shared/aim.c ../shared/aim.h ../config.h
	$(CC) $(CFLAGS) -c ../shared/aim.c

weapon.o: ../shared/weapon.c ../shared/weapon.h ../config.h
	$(CC) $(CFLAGS) -c ../shared/weapon.c

# 4. Alvo de limpeza
.PHONY: clean
clean:
	rm -f *.o $(TARGET)