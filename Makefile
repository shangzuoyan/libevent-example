CC = gcc

SOURCES ?= $(wildcard src/*.c)
OBJS    ?= $(addprefix , $(SOURCES:.c=.o))

CFLAGS += -W -Wall
CFLAGS += -O3 -g -fno-omit-frame-pointer
CFLAGS += -fno-strict-aliasing
CFLAGS += -I. -Iinclude/

LDFLAGS += -levent -lpthread

TARGET := socket

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "[BIN] $@"
	@$(CC) $^ $(LDFLAGS) -o $@

$(OBJS): %.o: %.c
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(OBJS) $(TARGET)

format:
	@astyle --style=linux -s4 -n src/*.c
	@astyle --style=linux -s4 -n include/*.h

