CC=gcc
CFLAGS=-I.
DEPS = ../crc/mbcrc.h ../cobsm/cobsm.h 
OBJ = main.c ../crc/mbcrc.c ../cobsm/cobsm.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

decode: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)