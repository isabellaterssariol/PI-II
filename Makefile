CC = gcc

SRC = src
BIN = jogo

CFLAGS = -Wall -I.
LIBS = $(shell pkg-config --cflags --libs allegro-5 allegro_image-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_audio-5 allegro_acodec-5)

OBJ = $(SRC)/main.o

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

$(SRC)/%.o: $(SRC)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	del /Q $(SRC)\*.o $(BIN).exe 2>nul || rm -f $(SRC)/*.o $(BIN)

run: all
	./$(BIN)
