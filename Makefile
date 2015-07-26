CFLAGS=
LDFLAGS=
LDLIBS=
C_FILES=$(wildcard src/*.c)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
TARGET=./bin/pilsner

all: $(OBJ_FILES)
	clang $(C_FILES) -O3 -I ./include -g -o $(TARGET) $(CFLAGS) $(LDFLAGS) $(LDLIBS)

run:
	$(TARGET)


