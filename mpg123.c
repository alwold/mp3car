#include "mpg123.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

extern struct mpg123_type mpg123;

struct mpg123_type mpg123_init( ) {
   int toprog[2];
   int fromprog[2];
   pid_t childpid;
   void *buf;
   struct mpg123_type prog;

   pipe(toprog);
   pipe(fromprog);

   childpid = fork();
   
   if ( childpid == 0 ) {
      close(toprog[1]);
      close(fromprog[0]);
      if ( dup2(toprog[0],0) == -1 ) {
         perror("dup2");
      }
      if ( dup2(fromprog[1],1) == -1 ) {
         perror("dup2");
      }
      execlp("/usr/bin/mpg123", "mpg123", "-R", ".", NULL);
      perror("execl");
      exit(0);
   } else {
      close(toprog[0]);
      close(fromprog[1]);
      prog.topipe = toprog[1];
      prog.frompipe = fromprog[0];
      prog.outlength = 0;
      prog.out[0] = '\0';
      prog.seconds = 0;
      prog.mpg123_pid = childpid;
      if ( fcntl(prog.frompipe, F_SETFL, O_NONBLOCK) == -1 ) {
         perror("fcntl");
      }
      read(prog.frompipe, buf, 10);
   }
   return prog;
}

void mpg123_cmd( struct mpg123_type prog, int cmd, char *arg)
{
   char *sendme;
   switch (cmd) {
      case CMD_PLAY:
         if ( mpg123.playstat == PLAY_PAUSE ) {
            sendme = (char *)malloc(strlen("PAUSE\n")+1);
            strcpy(sendme, "PAUSE\n");
         } else {
            sendme = (char *)malloc(7+strlen(arg));
            sprintf(sendme, "LOAD %s\n", arg);
         }
         mpg123.playstat = PLAY_PLAY;
         break;
      case CMD_STOP:
         sendme = (char *)malloc(6);
         strcpy(sendme, "STOP\n");
         break;
      case CMD_QUIT:
         sendme = (char *)malloc(6);
         strcpy(sendme, "QUIT\n");
         break;
      case CMD_JUMP:
         sendme = (char *)malloc(7+strlen(arg));
         sprintf(sendme, "JUMP %s\n", arg);
         break;
      case CMD_PAUSE:
         sendme = (char *)malloc(strlen("PAUSE\n")+1);
         strcpy(sendme, "PAUSE\n");
         break;
      default:
         printf("mpg123_cmd(): uknown command: %d\n", cmd);
         return;
      
   }
   write(prog.topipe, sendme, strlen(sendme));
}

struct mpg123_type mpg123_proc( struct mpg123_type prog ) {
   char buf[2], tmp[2];
   int i, field, len;
   char seconds[10], secondsleft[10], frames[10], framesleft[10];

   while ( read(prog.frompipe, buf, 1) == 1 ) {
      if ( buf[0] != '\n' ) {
         sprintf(prog.out, "%s%c", prog.out, buf[0]);
         prog.outlength++;
      } else {
         if ( prog.out[0] != '@' ) {
            printf("Weird command: %s", prog.out);
         } else {
            switch (prog.out[1]) {
               case 'R':
                  break;
               case 'I':
                  break;
               case 'S':
                  break;
               case 'F':
                  field = 0;
                  seconds[0] = '\0';
                  secondsleft[0] = '\0';
                  frames[0] = '\0';
                  len = strlen(prog.out);
                  for ( i = 3; i < len; i++ ) {
                     if ( prog.out[i] == ' ' ) {
                        field++;
                     } else {
                        if ( field == 0 ) {
                          sprintf(frames, "%s%c", frames, prog.out[i]);
                        }
                        if ( field == 1 ) {
                          sprintf(framesleft, "%s%c", framesleft, prog.out[i]);
                        }
                        if ( field == 2 ) {
                          sprintf(seconds, "%s%c", seconds,prog.out[i]);
                        }
                        if ( field == 3 ) {
                          sprintf(secondsleft, "%s%c", secondsleft,prog.out[i]);
                        }
                     }
                  }
                  prog.seconds = atof(seconds);
                  prog.secondsleft = atof(secondsleft);
                  prog.frames = atoi(frames);
                  prog.framesleft = atoi(framesleft);
                  break;
               case 'P':
                  tmp[0] = prog.out[3];
                  tmp[1] = '\0';
                  prog.playstat = atoi(tmp);
                  break;
               case 'E':
                  printf("Error: %s\n", prog.out);
                  break;
               default:
                  printf("unknown response: %s\n", prog.out);
                  break;
            }
         }
         prog.outlength = 0;
         prog.out[0] = '\0';
      }
   }
   return prog;
}
