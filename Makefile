CC      = gcc
CFLAGS  = -Wall -Wextra -O2
TARGET  = greenhouse_monitor
SRC     = greenhouse_monitor.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
