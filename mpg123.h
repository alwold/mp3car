#ifndef TYPES_H
#include <sys/types.h>
#endif

struct mpg123_type {
   int topipe;
   int frompipe;
   int outlength;
   char out[255];
   float seconds;
   float secondsleft;
   int frames;
   int framesleft;
   pid_t mpg123_pid;
   int playstat;
   float bar;
};

struct mpg123_type mpg123_init();

void mpg123_cmd( struct mpg123_type, int, char *);

struct mpg123_type mpg123_proc( struct mpg123_type );
#ifdef CMD_PLAY
#error CMD_PLAY defined
#endif

#define CMD_PLAY 1
#define CMD_STOP 2
#define CMD_QUIT 3
#define CMD_JUMP 4
#define CMD_PAUSE 5

#define PLAY_STOP 0
#define PLAY_PLAY 2
#define PLAY_PAUSE 1
