kissfft_dir := kissfft
kissfft := $(kissfft_dir)/libkissfft.so

rave: rave.c
	gcc -o rave rave.c -lm -lxcb -lxcb-randr -ggdb

$(kissfft):
	make -C kissfft

pulseaudio: pulseaudio.c $(kissfft)
	gcc -o pulseaudio pulseaudio.c $(kissfft) -lm -lpulse -lpulse-simple -ggdb -Wl,-R$(kissfft_dir)

clean:
	-rm rave pulseaudio