rave: rave.c
	gcc -o rave rave.c -lm -lxcb -lxcb-randr -ggdb

clean:
	rm rave