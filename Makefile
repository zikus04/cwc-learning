PKGS = wayland-server
FLAGS = `pkg-config --libs --cflags $(PKGS)` -Iinclude/

make:
	gcc -o cwc -g main.c $(FLAGS)
