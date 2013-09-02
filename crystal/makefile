canvas.o: canvas.c canvas.h color.h
	gcc -lpng -c canvas.c -o canvas.o

crystal: crystal.c canvas.o
	gcc -lpng canvas.o crystal.c -o crystal

simulate: crystal
	./crystal

.PHONY: clean
clean:
	rm -f canvas.o
	rm -f crystal
	rm -f log.txt
	rm -f *.png
