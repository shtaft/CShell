.phony all:
all: csc360.c
	gcc csc360.c -o csc360 -lreadline

.PHONY clean:
clean:
	-rm -rf *.o *.exe
