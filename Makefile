CFLAGS=
LDFLAGS=
LDLIBS=

all: main.o
	clang main.c gc.c obj.c error.c -g -o gcfool $(CFLAGS) $(LDFLAGS) $(LDLIBS)

run:
	./gcfool


