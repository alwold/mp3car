#include <gtk/gtk.h>
#include "playlist.h"
#include "mpg123.h"

extern struct mpg123_type mpg123;

extern GtkWidget *filename, *playinfo;

int pls_add( char *dir, char *file ) {
   char *row[2];
   struct pls_entry *node, *tmp;


   if ( !dir || !file ) {
      return 1;
   }
   if ( playlist.first ) {
      node = playlist.first;
      while(node->next) {
         node = node->next;
      }
      node->next = (struct pls_entry *)malloc(sizeof(struct pls_entry));
      tmp = node;
      node = node->next;
      node->prev = tmp;
      tmp = node->prev;
      // Cap the end
      node->next = 0;
   } else {
      playlist.first = (struct pls_entry *)malloc(sizeof(struct pls_entry));
      node = playlist.first;
      // Cap the ends
      node->prev = 0;
      node->next = 0;
   }
   node->file = (char *)malloc(strlen(file)+1);
   strcpy(node->file, file);
   node->dir = (char *)malloc(strlen(dir)+1);
   strcpy(node->dir, dir);

   // Parse out the artist and title
   parsefile(file, row);
   node->id = gtk_clist_append( GTK_CLIST(pls_clist), row);
   return 0;
}

gint delete_song( GtkWidget *w, gpointer data ) {
   pls_delete(playlist.selection);
   return 0;
}

gint clear_list( GtkWidget *w, gpointer data ) {
   pls_clear();
   return 0;
}

int pls_delete( int id ) {
   struct pls_entry *node, *tmp, *freeme;
   int renumber = 0;
   char songtxt[255];

   node = playlist.first;
   while ( node ) {
      if ( node->id == id && renumber == 0) {
         if ( playlist.current && playlist.current->id == id ) {
             if ( status.playing == 1 ) {
                mpg123_cmd(mpg123, CMD_STOP, NULL);
                playlist.current = 0;
                status.playing = 0;
                gtk_label_set_text(GTK_LABEL(playinfo), "Stopped");
             }
             gtk_label_set_text(GTK_LABEL(filename), "(none)");
         }
         freeme = node;
         if ( node->next ) {
            if ( node->prev ) {
               node = node->prev;
               node->next = node->next->next;
               node->next->prev = node;
               free(freeme);
            } else {
               node = node->next;
               playlist.first = node;
               node->prev = 0;
               free(freeme);
            }
         } else {
            if ( node->prev ) {
               node = node->prev;
               node->next = 0;
               free(freeme);
            } else {
               playlist.first = 0;
               free(freeme);
               node = 0;
            }
         }
         /*if (node->prev) {
            node = node->prev;
            if ( node->next ) {
               freeme = node->next;
               // May need to deep free this
               free(freeme);
            }
            node->next = tmp;
         } else {
            playlist.first = node->next;
            node = node->next;
            free(node->prev);
            node->prev = 0;
         }*/
         gtk_clist_remove( GTK_CLIST(pls_clist), id );
         renumber = 1;
         node = playlist.first;
         id = 0;
      } 
      if ( renumber == 1 && node ) {
         node->id = id;
         id++;
      }
      if ( node )
         node = node->next;
   }
}

gint select_row( GtkWidget *w, gint row, gint column, GdkEventButton *e,gpointer d) {
   playlist.selection = row;
   return 0;
}

void pls_dump() {
   struct pls_entry *node, *prenode;

   node = playlist.first;
   g_print("forward...\n");
   while ( node ) {
      g_print("ID: %d (%s)\n", node->id, node->file);
      prenode = node;
      node = node->next;
   }
   node = prenode;
   g_print("back...\n");
   while ( node ) {
      g_print("ID: %d (%s)\n", node->id, node->file);
      node = node->prev;
   }
}

void pls_clear() {
   struct pls_entry *node;
   if ( status.playing == 1 ) {
      mpg123_cmd(mpg123, CMD_STOP, NULL);
      status.playing = 0;
   }
   node = playlist.first;
   playlist.first = 0;
   playlist.current = 0;
   while ( node ) {
      if ( node->next ) {
         node = node->next;
         free(node->prev);
      } else {
         free(node);
         node = 0;
      }
   }
   gtk_clist_clear( GTK_CLIST(pls_clist) );
} 

void parsefile( const char *filename, char **entry ) {
   int i, flen, found = 0, index;
   char *filenamecp;
   filenamecp = (char *)malloc(strlen(filename)+1);
   strcpy(filenamecp, filename);
   flen = strlen(filenamecp);

   /* Strip off the extension */
   filenamecp[flen-4] = 0;
   flen = strlen(filenamecp);

   /* find the split of the artist/title */
   index = 0;
   for ( i = 0; i < flen; i++ ) {
      if ( filenamecp[i] == ' ' && found == 0 ) {
         found = 1;
      } else if ( filenamecp[i] == '-' && found == 1 ) {
         found = 2;
      } else if ( filenamecp[i] == ' ' && found == 2 ) {
         index = i-2;
      } else {
         found = 0;
      }
   }

   /* copy the data into the entry for the clist */
   entry[0] = (char *)malloc(index+1);
   strncpy(entry[0], filenamecp, index);
   entry[0][index] = 0;
   entry[1] = (char *)malloc(strlen(filenamecp)-(index+3)+1);
   strcpy(entry[1], filenamecp+(index+3));
}
