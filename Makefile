CC = gcc
CFLAGS = -g -Wall -Wvla -Werror -fsanitize=address,undefined

TARGET = mysh

SRC = mysh.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)