CC=clang
TARGET=font2png

CFLAGS=-Wall -Werror -Wextra -O0 -g \
	   `pkg-config --cflags pangocairo`
LDFLAGS=`pkg-config --libs pangocairo`

SOURCES=$(shell echo *.c)
OBJECTS=$(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS) .depend *.png

.depend: *.[ch]
	$(CC) -MM `pkg-config --cflags pangocairo` $(SOURCES) > .depend

-include .depend
.PHONY: clean all
