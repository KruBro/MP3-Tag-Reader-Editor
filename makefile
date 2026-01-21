OBJ = $(patsubst %.c, %.o, $(wildcard *.c))

mp3_tag_reader.out : $(OBJ)
	gcc -o $@ $^

clean :
	rm *.o *.out