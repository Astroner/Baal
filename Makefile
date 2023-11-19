CC=gcc
SOURCES:=$(wildcard src/*.c src/*/*.c)
OBJECTS=$(SOURCES:.c=.o)
HEADERS=headers
LIBNAME=Baal.h

EXECUTABLE=./start

all: build
	$(EXECUTABLE)

build: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $^

.c.o:
	$(CC) $(CFLAGS) -c -I$(HEADERS) -Wall -Wextra -o $@ $<

lib: headers/Baal.h src/Baal.c
	echo "/*" > $(LIBNAME)
	cat README.md >> $(LIBNAME)
	echo "*/\n\n" >> $(LIBNAME)
	cat headers/Baal.h >> $(LIBNAME)
	echo "#if defined(BAAL_IMPLEMENTATION)" >> $(LIBNAME)
	tail -n +2 src/Baal.c >> $(LIBNAME)
	echo "\n#endif" >> $(LIBNAME)

test: lib
	$(CC) -o test test.c
	./test
	rm -f test

clean:
	rm -f $(EXECUTABLE) $(OBJECTS) $(LIBNAME)