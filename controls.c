#include <gtk/gtk.h>
#include "controls.h"
#include "playlist.h"
#include "mpg123.h"

extern struct mpg123_type mpg123;

extern GtkWidget *filename, *playinfo;

extern char *mp3dir;

gint position_handler( GtkAdjustment *w, gpointer d ) {
   g_print("Adjusted to %f\n", w->value);
   if ( (int)w->value != (int)mpg123.bar ) {
      //mpg123_cmd(mpg123, CMD_JUMP, "+50");
      g_print("ne\n");
   }
}

gint rewff_handler( GtkWidget *w, char *a ) {
   char cmd[255];
   sprintf(cmd, "%s%i", a, ((mpg123.frames)/(mpg123.seconds+mpg123.secondsleft))*3);
   g_print("adjusting %s.  There are %i frames and %f seconds in this song.\n", cmd, mpg123.frames, mpg123.seconds+mpg123.secondsleft);
   mpg123_cmd(mpg123, CMD_JUMP, cmd);
   return 0;
}

gint play_handler( GtkWidget *widg, gpointer data ) {
   char cmd[255], songtxt[255];

   if ( ! playlist.current ) {
      if ( ! playlist.first ) {
         return 0;
      } else {
         playlist.current = playlist.first;
         /* update the info display */
         sprintf(songtxt, "%s", playlist.current->file);
         gtk_label_set_text( GTK_LABEL(filename), songtxt);
      }
   }
   sprintf(cmd, "%s/%s","/mp3",playlist.current->file);
   mpg123_cmd(mpg123, CMD_PLAY, cmd);
   status.playing = 1;
   status.seconds = -1;
}

gint pause_handler( GtkWidget *w, gpointer d ) {
   status.playing = PLAY_PAUSE;
   mpg123_cmd(mpg123, CMD_PAUSE, NULL);
   gtk_label_set_text(GTK_LABEL(playinfo), "Paused");
   return 0;
}

gint stop_handler( GtkWidget *widg, gpointer data ) {
   status.playing = 0;
   mpg123_cmd(mpg123, CMD_STOP, NULL);
   gtk_label_set_text(GTK_LABEL(playinfo), "Stopped");
   return 0;
}

gint next_handler( GtkWidget *widget, gpointer data ) {
   char songtxt[255];

   if ( !playlist.current ) {
      playlist.current = playlist.first;
   }
   if ( playlist.current->next != 0 ) {
      playlist.current = playlist.current->next;
      /* update display */
      sprintf(songtxt, "%s", playlist.current->file);
      gtk_label_set_text( GTK_LABEL(filename), songtxt);
   }
   if ( status.playing == 1 ) {
      play_handler(NULL, NULL);
   }
   return 0;
}

gint prev_handler( GtkWidget *widget, gpointer data ) {
   char songtxt[255];

   if ( !playlist.current ) {
      playlist.current = playlist.first;
   }
   if ( playlist.current->prev != 0 ) {
      playlist.current = playlist.current->prev;
      /* update display */
      sprintf(songtxt, "%s", playlist.current->file);
      gtk_label_set_text( GTK_LABEL(filename), songtxt);
   }
   if ( status.playing == 1 ) {
      play_handler(NULL, NULL);
   }
   return 0;


}
