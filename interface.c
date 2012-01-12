#include <gtk/gtk.h>
#include <signal.h>
#include "dirlist.h"
#include "playlist.h"
#include "controls.h"
#include "mpg123.h"
#include "pixbutton.h"
#include "load_pls.h"

struct mpg123_type mpg123;

GtkWidget *window, *filename, *playinfo, *songbar, *notebook;
GtkObject *songadj;
char *mp3dir;

void mpplay( GtkWidget *, gpointer );

void playlist_load( GtkWidget *, gpointer );

gint mainquit( gpointer d) {
   gtk_main_quit();
   return FALSE;
}
void do_demo_screen() {
   GtkWidget *window, *pixmap;
   GtkStyle *style;
   GdkPixmap *pix;
   GdkBitmap *mask;
   gint tag;

   window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
   gtk_widget_set_usize( GTK_WIDGET (window), 640, 400);
   gtk_widget_realize(window);
   style = gtk_widget_get_style(window);
   pix = gdk_pixmap_create_from_xpm(window->window, &mask,
                                    &style->bg[GTK_STATE_NORMAL],
                                    "/img/uas.xpm");
   pixmap = gtk_pixmap_new(pix, mask);
   gtk_widget_show(pixmap);
   gtk_container_add( GTK_CONTAINER(window), pixmap);
   gtk_widget_show(window);
   tag = gtk_timeout_add( 3000, mainquit, NULL);
   gtk_main();
   gtk_widget_hide(window);
   //gtk_timeout_remove(tag);
}
   
// If the child (mpg123) dies, this function will respawn it
void sigchild( int hmm ) {
   mpg123_init(mpg123);
}

// Function to handle data on the pipe from mpg123 (Updates the display
// and stuff like that)
void pipeready( gpointer data, gint source, GdkInputCondition c ) {
   char playstat[25], songname[255];
   int seconds, minutes;

   mpg123 = mpg123_proc(mpg123);
   if ( gtk_notebook_get_current_page( GTK_NOTEBOOK(notebook) ) == 0 ) {
      if ( status.playing == 1 && status.seconds != (int)mpg123.seconds) {
         status.seconds = (int)mpg123.seconds;
         minutes = status.seconds/60;
         seconds = status.seconds - (minutes*60);
         sprintf(playstat, "Playing - [%d:%02d]", minutes, seconds);
         gtk_label_set_text(GTK_LABEL(playinfo), playstat);
      gtk_adjustment_set_value(GTK_ADJUSTMENT(songadj), 100*(mpg123.seconds/(mpg123.secondsleft+mpg123.seconds)));
         mpg123.bar = 100*(mpg123.seconds/(mpg123.secondsleft+mpg123.seconds));
      }
      //gtk_progress_bar_update( GTK_PROGRESS_BAR(songbar),mpg123.seconds/(mpg123.secondsleft+mpg123.seconds));
   }
   if ( mpg123.playstat == 0 && status.playing == 1 ) {
      playlist.current = playlist.current->next;
      play_handler(NULL, NULL);
      sprintf(songname, "%s", playlist.current->file);
      gtk_label_set_text( GTK_LABEL(filename), songname );
   }
}

// This function will clean up the mpg123 process when the program
// is terminated.  mpg123 tends to go crazy if the pipe breaks and 
// you don't kill it.
gint cleanup( GtkWidget *w, GdkEvent *e, gpointer d ) {
   signal(SIGCHLD, SIG_DFL);
   mpg123_cmd(mpg123, CMD_QUIT, NULL);
   kill(SIGKILL, mpg123.mpg123_pid);
   gtk_main_quit();
}

// This function draws the page on the main interface which has a list of
// playlists that can be loaded.
GtkWidget *draw_playlists( void ) {
   GtkWidget *list, *scroller, **filebuttons, *labelbox, *label;
   struct playlists *plslist;
   int i;

   list = gtk_vbox_new(TRUE, 0);
   scroller = gtk_scrolled_window_new(NULL, NULL);
   gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(scroller), list);

   /* get playlists and make buttons */
   plslist = getpls("/pls");
   filebuttons = (GtkWidget **)malloc(sizeof(GtkWidget *)*plslist->count);
   for( i = 0; i < plslist->count; i++ ) {
      filebuttons[i] = gtk_button_new();
      labelbox = gtk_hbox_new(FALSE, 0);
      label = gtk_label_new(plslist->filenames[i]);
      gtk_box_pack_start( GTK_BOX(labelbox), label, FALSE, FALSE, 0);
      gtk_label_set_justify( GTK_LABEL(label), GTK_JUSTIFY_LEFT);
      gtk_container_add( GTK_CONTAINER(filebuttons[i]), labelbox);
      gtk_box_pack_start( GTK_BOX (list), filebuttons[i], FALSE, FALSE, 0 );
      gtk_signal_connect( GTK_OBJECT(filebuttons[i]), "clicked",GTK_SIGNAL_FUNC(playlist_load), (gpointer)plslist->filenames[i] );
      gtk_widget_show( GTK_WIDGET(filebuttons[i]) );
      gtk_widget_show( label );
      gtk_widget_show( labelbox );
   }      
   gtk_widget_show(list);
   gtk_widget_show(scroller);
   return scroller;
}

// This function draws the main controls page with the song status, play
// buttons, etc...
GtkWidget *draw_controls( void ) {
   GtkWidget *control_hbox, *control_vbox, *fileinfo_vbox, *button_play, *button_rew, *button_ff, *button_prev, *button_next, *button_stop, *button_pause;
   char *filenamestr;

   control_vbox = gtk_vbox_new(FALSE, 0);
   control_hbox = gtk_hbox_new(TRUE, 0);
   button_play = gtk_button_new_with_pixmap(window, "/img/play.xpm");
   gtk_signal_connect( GTK_OBJECT(button_play), "clicked", GTK_SIGNAL_FUNC(play_handler), NULL);
   button_rew = gtk_button_new_with_pixmap(window, "/img/rew.xpm");
   button_ff = gtk_button_new_with_pixmap(window, "/img/ff.xpm");
   gtk_signal_connect( GTK_OBJECT(button_rew), "clicked", GTK_SIGNAL_FUNC(rewff_handler), "-");
   gtk_signal_connect( GTK_OBJECT(button_ff), "clicked", GTK_SIGNAL_FUNC(rewff_handler), "+");
   button_stop = gtk_button_new_with_pixmap(window, "/img/stop.xpm");
   gtk_signal_connect( GTK_OBJECT(button_stop), "clicked", GTK_SIGNAL_FUNC(stop_handler), NULL);
   button_pause = gtk_button_new_with_pixmap(window, "/img/pause.xpm");
   gtk_signal_connect( GTK_OBJECT(button_pause), "clicked", GTK_SIGNAL_FUNC(pause_handler), NULL);
   button_prev = gtk_button_new_with_pixmap(window, "/img/prev.xpm");
   gtk_signal_connect( GTK_OBJECT(button_prev), "clicked", GTK_SIGNAL_FUNC(prev_handler), NULL);
   button_next = gtk_button_new_with_pixmap(window, "/img/next.xpm");
   gtk_signal_connect( GTK_OBJECT(button_next), "clicked", GTK_SIGNAL_FUNC(next_handler), NULL);
   gtk_box_pack_start( GTK_BOX(control_hbox), button_prev, TRUE, TRUE, 0);
   gtk_box_pack_start( GTK_BOX(control_hbox), button_rew, TRUE, TRUE, 0);
   gtk_box_pack_start( GTK_BOX(control_hbox), button_stop, TRUE, TRUE, 0);
   gtk_box_pack_start( GTK_BOX(control_hbox), button_play, TRUE, TRUE, 0);
   gtk_box_pack_start( GTK_BOX(control_hbox), button_pause, TRUE, TRUE, 0);
   gtk_box_pack_start( GTK_BOX(control_hbox), button_ff, TRUE, TRUE, 0);
   gtk_box_pack_start( GTK_BOX(control_hbox), button_next, TRUE, TRUE, 0);
   gtk_widget_show( GTK_WIDGET(button_prev) );
   gtk_widget_show( GTK_WIDGET(button_rew) );
   gtk_widget_show( GTK_WIDGET(button_stop) );
   gtk_widget_show( GTK_WIDGET(button_play) );
   gtk_widget_show( GTK_WIDGET(button_pause) );
   gtk_widget_show( GTK_WIDGET(button_ff) );
   gtk_widget_show( GTK_WIDGET(button_next) );
   fileinfo_vbox = gtk_vbox_new(TRUE, 0);
   filename = gtk_label_new("(none)");
   gtk_box_pack_start(GTK_BOX(fileinfo_vbox), filename, TRUE, TRUE, 0);
   gtk_widget_show( filename );
   
   //songbar = gtk_progress_bar_new();
   songadj = gtk_adjustment_new(0.0, 0.0, 100.0, 1.0, 5.0, 5.0);
   songbar = gtk_hscale_new(GTK_ADJUSTMENT(songadj));
   gtk_widget_set_usize( songbar, 400, 75);
   gtk_scale_set_draw_value( GTK_SCALE(songbar), FALSE);
   //gtk_signal_connect( songbar, "drag-end", 
                       //GTK_SIGNAL_FUNC(position_handler), NULL);
   
   gtk_box_pack_start(GTK_BOX(fileinfo_vbox), songbar, TRUE, TRUE, 0);
   gtk_widget_show( songbar );
   playinfo = gtk_label_new("Stopped");
   gtk_box_pack_start(GTK_BOX(fileinfo_vbox), playinfo, TRUE, TRUE, 0);
   gtk_widget_show( playinfo );
   gtk_widget_show( GTK_WIDGET(fileinfo_vbox) );
   gtk_box_pack_start(GTK_BOX(control_vbox), fileinfo_vbox, TRUE, TRUE, 10);
   gtk_box_pack_start(GTK_BOX(control_vbox), control_hbox, FALSE, TRUE, 10);
   gtk_widget_show( GTK_WIDGET(control_hbox) );

   return control_vbox;
}

// This function starts playing a song when the play button is pressed
// or the track is changed while playing.
void mpplay( GtkWidget *widg, gpointer song ) {
   /*char *artist, *title;
   int i = 0;
   while (((char *)song)[i] != '-') {
      i++;
   }
   i-=1;
   artist = (char *)malloc(i+1);
   strncpy(artist,song,i);
   artist[i] = '\0';
   title = (char *)malloc(strlen(song)-i-6);
   strncpy(title,(song)+i+3,strlen(song)-i-7);
   title[strlen(song)-i-7] = '\0';*/
   pls_add("/mp3", song);
}

void playlist_load( GtkWidget *w, gpointer pls ) {
   char plsfile[255];
   sprintf(plsfile, "/pls/%s", pls);
   load_pls(plsfile);
}
// This function is overloaded with crap and needs to be cleaned up.
int main( int argc, char *argv[]) {
   GtkWidget *scroller, *vbox, **buttons, *labelbox, *label, *label1, 
             *label2, *label3, *label4, *pls_hbox, *pls_scroller, *playlists,
             *pls_button1, *pls_button2, *pls_vbox, *control_page;
   struct songs *songlist;
   char *clist_titles[] = {"Artist Name             ", "Song Name"};
   int i;

   mp3dir = argv[1];
   gtk_init(&argc, &argv);
   gtk_rc_parse("./gtkrc.mp3car");
   do_demo_screen();
   window = gtk_window_new( GTK_WINDOW_TOPLEVEL);
   gtk_widget_set_usize( GTK_WIDGET (window), 640, 400);

   gtk_widget_realize(window);
   // Make notebook and make some tabs
   notebook = gtk_notebook_new();
   
   label1 = gtk_label_new("   Control    ");
   label2 = gtk_label_new("   Playlist   ");
   label3 = gtk_label_new("  File List   ");
   label4 = gtk_label_new("  Playlists   ");
   /*label4 = gtk_label_new("Shutdown");*/

   // Setup the playlist boxes
   pls_hbox = gtk_hbox_new(TRUE, 0);
   pls_button1 = gtk_button_new_with_label("Delete");
   gtk_signal_connect(GTK_OBJECT(pls_button1),"clicked",GTK_SIGNAL_FUNC(delete_song),NULL);
   pls_button2 = gtk_button_new_with_label("Clear Playlist");
   gtk_signal_connect(GTK_OBJECT(pls_button2), "clicked", GTK_SIGNAL_FUNC(clear_list), NULL);
   gtk_box_pack_start(GTK_BOX(pls_hbox), pls_button1, TRUE, TRUE, 0);
   gtk_widget_show( GTK_WIDGET(pls_button1) );
   gtk_box_pack_start(GTK_BOX(pls_hbox), pls_button2, TRUE, TRUE, 0);
   gtk_widget_show( GTK_WIDGET(pls_button2) );
   pls_scroller = gtk_scrolled_window_new(NULL, NULL);
   pls_clist = gtk_clist_new_with_titles(2, clist_titles);
   gtk_signal_connect(GTK_OBJECT(pls_clist), "select_row", GTK_SIGNAL_FUNC(select_row), NULL);
   pls_vbox = gtk_vbox_new(FALSE, 0);
   scroller = gtk_scrolled_window_new(NULL, NULL);
   gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(pls_scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
   gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(pls_scroller), pls_clist);
   gtk_box_pack_start(GTK_BOX(pls_vbox), pls_scroller, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(pls_vbox), pls_hbox, FALSE, TRUE, 0);

   // Draw playlists page
   playlists = draw_playlists();
   // Draw controls page
   control_page = draw_controls();


   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), control_page, label1);
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), pls_vbox, label2);
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroller, label3);
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), playlists, label4);
   gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW (scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

   gtk_container_add( GTK_CONTAINER(window), notebook);


   vbox = gtk_vbox_new(TRUE, 0);
   gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(scroller), vbox);
   /* get songs and make buttons */
   songlist = getsongs("/mp3");
   buttons = (GtkWidget **)malloc(sizeof(GtkWidget *)*songlist->count);
   for( i = 0; i < songlist->count; i++ ) {
      buttons[i] = gtk_button_new();
      labelbox = gtk_hbox_new(FALSE, 0);
      label = gtk_label_new(songlist->filenames[i]);
      gtk_box_pack_start( GTK_BOX(labelbox), label, FALSE, FALSE, 0);
      gtk_label_set_justify( GTK_LABEL(label), GTK_JUSTIFY_LEFT);
      gtk_container_add( GTK_CONTAINER(buttons[i]), labelbox);
      gtk_box_pack_start (GTK_BOX (vbox), buttons[i], FALSE, FALSE, 0);
      gtk_signal_connect( GTK_OBJECT(buttons[i]), "clicked",GTK_SIGNAL_FUNC(mpplay), (gpointer)songlist->filenames[i]);
      gtk_widget_show( GTK_WIDGET(buttons[i]));
      gtk_widget_show( label);
      gtk_widget_show( labelbox);
   }      
   gtk_widget_show( vbox );
   gtk_widget_show( scroller );
   gtk_widget_show( playlists );
   gtk_widget_show( control_page );
   gtk_widget_show( pls_clist );
   gtk_widget_show( pls_scroller );
   gtk_widget_show( pls_hbox );
   gtk_widget_show( pls_vbox );
   gtk_widget_show( notebook );
   gtk_notebook_set_page( GTK_NOTEBOOK(notebook), 0);
   gtk_widget_show( window );

   mpg123 = mpg123_init();

   signal(SIGCHLD, sigchild);

   gtk_signal_connect(GTK_OBJECT(window), "delete_event",
      GTK_SIGNAL_FUNC(cleanup), NULL);
   gtk_signal_connect(GTK_OBJECT(window), "destroy",
      GTK_SIGNAL_FUNC(cleanup), NULL);

   gdk_input_add( mpg123.frompipe, GDK_INPUT_READ, pipeready, NULL );
   gtk_main();
   return 0;

}
