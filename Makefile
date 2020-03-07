kissfft_dir := kissfft
kissfft := $(kissfft_dir)/libkissfft.so

rave: rave.c
	gcc -o rave rave.c -lm -lxcb -lxcb-randr -ggdb

$(kissfft):
	make -C kissfft

alsa: alsa.c $(kissfft)
	gcc -o alsa alsa.c $(kissfft) -lasound -ggdb -Wl,-R$(kissfft_dir)

clean:
	-rm rave alsa