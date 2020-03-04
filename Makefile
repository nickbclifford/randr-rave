rave: rave.c
	gcc -o rave rave.c -lm -lxcb -lxcb-randr -ggdb

alsa: alsa.c
	gcc -o alsa alsa.c -lasound -ggdb

clean:
	-rm rave alsa