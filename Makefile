CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -O2 -D_POSIX_C_SOURCE=200809L
LDLIBS := -pthread -lrt
TARGET := campuscompute
SOURCES := src/main.c src/scheduler.c src/logger.c src/ipc.c src/banker.c src/workers.c
OBJECTS := $(SOURCES:.c=.o)

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

src/%.o: src/%.c src/campuscompute.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) campuscompute.log
