CFLAGS=
LDFLAGS=
LDLIBS=

all: main.o
	clang main.c -g -o gcfool $(CFLAGS) $(LDFLAGS) $(LDLIBS)

run:
	./gcfool


