canvas.o: canvas.c canvas.h color.h
	gcc -lpng -c canvas.c -o canvas.o

crystal: crystal.c canvas.o
	gcc -lpng canvas.o crystal.c -o crystal

simulate: crystal
	./crystal

video_simulate: simulate
	ffmpeg -r 5 -i step_%06d.png -c:v libx264 -pix_fmt yuv420p out.mp4

.PHONY: clean
clean:
	rm -f canvas.o
	rm -f crystal
	rm -f log.txt
	rm -f *.png
	rm -f *.mp4