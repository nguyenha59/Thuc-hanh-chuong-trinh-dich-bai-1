CC = gcc
CFLAGS = -g -Wall

TARGET = thuchanh1.exe
SRC = thuchanh1.c

# Default target
all: $(TARGET)
# Build rule
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
# Clean build files
clean:
	del $(TARGET)

