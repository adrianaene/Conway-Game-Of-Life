CC = gcc
CFLAGS = -fopenmp
SRC = tema1.c
EXE = tema1
 
all:
	$(CC) -o $(EXE) $(SRC) $(CFLAGS)
 
.PHONY : clean
clean :
	rm -f $(EXE) *~
