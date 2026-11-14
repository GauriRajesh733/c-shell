CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -std=c99 -D_GNU_SOURCE -fsanitize=address,undefined
TARGET = gauri-shell
SRC = shell.c pipevector.c stringvector.c tokenizer.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

test: $(TARGET)
	python3 tests/test_shell.py

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes --error-exitcode=1 ./$(TARGET)
