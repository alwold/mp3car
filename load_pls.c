#include "playlist.h"
#include "load_pls.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

// load_pls is a really crazy routine to read a playlist in from a file.
// Currently, it is limited to 255 character lines
void load_pls( char *plsname ) {
   FILE *pls;
   char *bytes, line[255], filename[255], *artist, *title, *tmp;
   int bcount, i, j, l, valid = 0;
   struct stat *pls_stat;

   pls_stat = (struct stat *)malloc(sizeof(struct stat));
   if ( stat(plsname, pls_stat) == -1 ) {
      perror("stat");
      return;
   }

   pls = fopen(plsname, "r");
   bytes = (char *)malloc(pls_stat->st_size);
   artist = (char *)malloc(80);
   title = (char *)malloc(79);
   tmp = (char *)malloc(80);
   bcount = fread(bytes, 1, pls_stat->st_size, pls);
   j = 0;
   pls_clear();
   for ( i = 0; i < bcount; i++ ) {
      if ( bytes[i] == '\n' || bytes[i] == '\r' ) {
         // Process a line
         if ( strcmp(line, "[playlist]") == 0 ) {
            valid = 1;
         }
         if ( valid && strncmp(line, "File", 4) == 0 ) {
            for ( l = 0; line[l] != '='; l++ );
            l++;
            sprintf(filename, "%s", line+l);
            //artist = strtok(filename, "-");
            artist[strlen(artist)-1] = '\0';
            //tmp = strtok(NULL, "");
            title = tmp+1;
            title[strlen(title)-4] = '\0';
            pls_add("/mp3", filename);
         }
         j = 0;
         line[j] = '\0';
      } else {
         line[j] = bytes[i];
         line[j+1] = '\0';
         j++;
      }
   }
}   
