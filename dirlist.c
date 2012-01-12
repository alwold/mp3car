#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "dirlist.h"
#include <stdio.h>

struct songs *getsongs( char *path ) {
   DIR *dirpath;
   struct dirent *directory;
   struct songs *songlist;
   int i,j,k, idx, pos;
   char buf[255];

   songlist = (struct songs *)malloc(sizeof(struct songs));
   dirpath = opendir(path);
   i = 0;
   songlist->count = 0;
   while( directory = readdir(dirpath)) {
      if ( directory->d_name[strlen(directory->d_name)-3] == 'm' &&
           directory->d_name[strlen(directory->d_name)-2] == 'p' &&
           directory->d_name[strlen(directory->d_name)-1] == '3' ) {
           /* store filename */
           pos = i;
           while(pos>0 && strcmp(songlist->filenames[pos-1],directory->d_name) > 0 ) {
              songlist->filenames[pos] = songlist->filenames[pos-1];
              pos--;
           }
           songlist->filenames[pos] = (char *)malloc(strlen(directory->d_name)+1);
           strcpy(songlist->filenames[pos],directory->d_name);

           /* find index of - */
           for (j = 0; j < strlen(directory->d_name); j++ ) {
              if ( directory->d_name[j+1] == ' ' && directory->d_name[j+2] == '-' ) {
                 idx = j;
                 break;
              }
           }
           /* store artist name */
           songlist->artists[pos] = (char *)malloc(idx+2);
           strncpy(songlist->artists[pos], directory->d_name, idx+1);
           songlist->artists[i+1] = '\0';

           /* store song name */
           k = 0;
           j = idx + 4;
           while( directory->d_name[j] != '.') {
              buf[k] = directory->d_name[j];
              j++;
              k++;
           }
           buf[k] = '\0';
           songlist->names[pos] = (char *)malloc(strlen(buf)+1);
           strcpy(songlist->names[pos], buf);
           /* increment counters */
           i++;
           songlist->count++;
           
      }
   }
   return songlist;
}
  
struct playlists *getpls( char *path ) {
   DIR *dirpath;
   struct dirent *directory;
   struct playlists *plslist;
   int i,j,k, idx, pos;
   char buf[255];

   plslist = (struct playlists *)malloc(sizeof(struct playlists));
   dirpath = opendir(path);
   i = 0;
   plslist->count = 0;
   while( directory = readdir(dirpath)) {
      if ( directory->d_name[strlen(directory->d_name)-3] == 'p' &&
           directory->d_name[strlen(directory->d_name)-2] == 'l' &&
           directory->d_name[strlen(directory->d_name)-1] == 's' ) {
           /* store filename */
           pos = i;
           while(pos>0 && strcmp(plslist->filenames[pos-1],directory->d_name) > 0 ) {
              plslist->filenames[pos] = plslist->filenames[pos-1];
              pos--;
           }
           plslist->filenames[pos] = (char *)malloc(strlen(directory->d_name)+1);
           strcpy(plslist->filenames[pos],directory->d_name);
           plslist->count++;
           i++;
      }
   }
   return plslist;
}
  
