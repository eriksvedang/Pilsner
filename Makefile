CFLAGS=
LDFLAGS=
LDLIBS=
FILES=main.c gc.c obj.c error.c parser.c runtime.c

all: main.o
	clang $(FILES) -g -o pilsner $(CFLAGS) $(LDFLAGS) $(LDLIBS)

run:
	./gcfool


