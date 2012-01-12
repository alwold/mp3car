CFLAGS=`gtk-config --cflags`
LDFLAGS=`gtk-config --libs`

all: objects link

objects: pixbutton.o interface.o dirlist.o playlist.o controls.o mpg123.o load_pls.o

link:
	$(CC) -o mp3 pixbutton.o interface.o dirlist.o playlist.o controls.o mpg123.o load_pls.o `gtk-config --libs`
