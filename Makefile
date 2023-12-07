CC=gcc-13
LIB_SOURCES=$(wildcard src/*.c)
DEV_SOURCES=$(LIB_SOURCES) $(wildcard dev/*.c)
OBJECTS=$(DEV_SOURCES:.c=.o)
LIBNAME=Baal.h

DEV_EXECUTABLE=./start

all: $(DEV_EXECUTABLE)
	$(DEV_EXECUTABLE)

$(DEV_EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(DEV_EXECUTABLE) $^

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c -std=c99 -pedantic -Iheaders -Wall -Wextra -o $@ $<

lib: headers/Baal.h headers/Baal-Defines.h $(LIB_SOURCES)
	cat headers/Baal.h > $(LIBNAME)
	echo "#if defined(BAAL_IMPLEMENTATION)" >> $(LIBNAME)
	cat headers/Baal-Defines.h >> $(LIBNAME)
	$(foreach srcFile, $(LIB_SOURCES), tail -n +3 $(srcFile) >> $(LIBNAME);)
	echo "\n#endif" >> $(LIBNAME)

tests: lib
	$(CC) -o run -Wall -Wextra -std=c99 -pedantic ./tests/main.c
	./run
	rm -f run

clean:
	rm -f $(DEV_EXECUTABLE) $(OBJECTS) $(LIBNAME)