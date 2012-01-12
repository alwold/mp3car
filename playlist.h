#ifndef GTK_H
#include <gtk/gtk.h>
#endif

struct pls_entry {
   int id;
   struct pls_entry *next;
   struct pls_entry *prev;
   char *file;
   char *dir;
};

struct pls {
   int songcount;
   struct pls_entry *first;
   struct pls_entry *current;
   int selection;
};

struct globs {
   int playing;
   int seconds;
};

int pls_add( char *, char * );

int pls_delete( int );

struct pls playlist;

GtkWidget *pls_clist;

struct globs status;

gint delete_song( GtkWidget *, gpointer );

gint clear_list( GtkWidget *, gpointer );

gint select_row(GtkWidget *, gint, gint, GdkEventButton *, gpointer);

void pls_clear( void );

void pls_dump( void );

void parsefile( const char *, char ** );
