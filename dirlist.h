struct songs {
   char *names[255];
   char *artists[255];
   char *filenames[255];
   int count;
};

struct playlists {
   char *filenames[255];
   int count;
};

struct songs *getsongs( char *path );
struct playlists *getpls( char *path );
