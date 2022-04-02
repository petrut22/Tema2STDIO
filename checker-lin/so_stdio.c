#include "so_stdio.h"
#include <sys/types.h>	/* open */
#include <sys/stat.h>	/* open */
#include <fcntl.h>	/* O_RDWR, O_CREAT, O_TRUNC, O_WRONLY */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>	/* close, lseek, read, write */
#define BUFFER_SIZE 4096
typedef struct _so_file {
    int fd;
    int flags;
    int cursor;
    int bytesRead;
    char buffer[BUFFER_SIZE];

} SO_FILE;


SO_FILE *so_fopen(const char *pathname, const char *mode) {
    int fd1 = -1;
    SO_FILE *file = (SO_FILE *)malloc(sizeof(SO_FILE));
    file->bytesRead = 0;
    file->cursor = 0;
    if(strcmp(mode, "r") == 0) {
        	fd1 = open(pathname, O_RDONLY);
            file->flags = O_RDONLY;
            file->fd = fd1;
    }

    if(strcmp(mode, "r+") == 0) {
            fd1 = open(pathname, O_RDWR); 
            file->flags = O_RDWR;
            file->fd = fd1;
    }

    if(strcmp(mode, "w") == 0) {
            fd1 = open(pathname, O_WRONLY|O_CREAT|O_TRUNC); 
            file->flags = O_WRONLY|O_CREAT|O_TRUNC;
            file->fd = fd1;       
    }

    if(strcmp(mode, "w+") == 0) {
            fd1 = open(pathname, O_RDWR|O_CREAT|O_TRUNC); 
            file->flags = O_RDWR|O_CREAT|O_TRUNC;
            file->fd = fd1;
    }

    if(strcmp(mode, "a") == 0) {
            fd1 = open(pathname, O_APPEND|O_CREAT); 
            file->flags = O_APPEND|O_CREAT ;
            file->fd = fd1;
    }

    if(strcmp(mode, "a+") == 0) {
            fd1 = open(pathname, O_APPEND|O_CREAT|O_RDONLY); 
            file->flags = O_APPEND|O_CREAT|O_RDONLY;
            file->fd = fd1;
    }

    if(fd1 < 0) {
        free(file);
        return NULL;
    }

    return file;

}


int so_fclose(SO_FILE *stream) {
    int rc;
    rc = close(stream->fd);
    if(rc < 0) {
        return SO_EOF;
    }
    free(stream);
    return 0;
}

int so_fgetc(SO_FILE *stream) {
   int size;
   unsigned char ch;
   if(stream->bytesRead == stream->cursor) {
        size = read(stream->fd, stream->buffer, BUFFER_SIZE);
        if(size <= 0) {
            return SO_EOF;
        }
        stream->bytesRead = size;
   }  

    printf("%d\n", stream->bytesRead);
    printf("%s\n", stream->buffer);
    printf("%d\n", stream->fd);

    //printf("%c %d\n",stream->buffer[stream->cursor], (int)stream->buffer[stream->cursor]);
    ch = stream->buffer[stream->cursor];
    (stream->cursor)++;
    return ch;
}


int so_fputc(int c, SO_FILE *stream){
    return -1;
}


size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream){
    return -1;
}


size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream){
    return -1;
}


int so_fseek(SO_FILE *stream, long offset, int whence){
    return -1;
}

long so_ftell(SO_FILE *stream){
    return -1;
}


int so_fflush(SO_FILE *stream){
    return -1;
}

int so_fileno(SO_FILE *stream){
    return -1;
}


int so_feof(SO_FILE *stream){
    return -1;
}


int so_ferror(SO_FILE *stream){
    return -1;
}


SO_FILE *so_popen(const char *command, const char *type){
    return NULL;
}


int so_pclose(SO_FILE *stream){
    return -1;
}
