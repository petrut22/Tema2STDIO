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
	int cursor;
	int bytesRead;
	int isWritting;
	int error;
	int cursorFile;
	int endOfFile;
	char buffer[BUFFER_SIZE];
} SO_FILE;

ssize_t xwrite(int fd, const void *buf, size_t count)
{
	size_t bytes_written = 0;

	while (bytes_written < count) {
		ssize_t bytes_written_now = write(fd, buf + bytes_written, count - bytes_written);

		if (bytes_written_now <= 0) /* I/O error */
			return -1;

		bytes_written += bytes_written_now;
	}

	return bytes_written;
}

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	int fd1 = -1;
	SO_FILE *file = (SO_FILE *)malloc(sizeof(SO_FILE));

	file->bytesRead = 0;
	file->cursor = 0;
	file->isWritting = 0;
	file->error = 0;
	file->cursorFile = 0;
	file->endOfFile = 0;

	if (strcmp(mode, "r") == 0) {
		fd1 = open(pathname, O_RDONLY);
		file->fd = fd1;
	}

	if (strcmp(mode, "r+") == 0) {
		fd1 = open(pathname, O_RDWR);
		file->fd = fd1;
	}

	if (strcmp(mode, "w") == 0) {
		fd1 = open(pathname, O_WRONLY|O_CREAT|O_TRUNC);
		file->fd = fd1;
	}

	if (strcmp(mode, "w+") == 0) {
		fd1 = open(pathname, O_RDWR|O_CREAT|O_TRUNC);
		file->fd = fd1;
	}

	if (strcmp(mode, "a") == 0) {
		fd1 = open(pathname, O_CREAT|O_APPEND|O_WRONLY);
		file->fd = fd1;
	}

	if (strcmp(mode, "a+") == 0) {
		fd1 = open(pathname, O_APPEND|O_CREAT|O_RDWR);
		file->fd = fd1;
	}

	if (fd1 < 0) {
		free(file);
		return NULL;
	}

	return file;

}


int so_fclose(SO_FILE *stream)
{
	int rc, index_fflush;

	index_fflush = so_fflush(stream);
	if (index_fflush < 0) {
		free(stream);
		return SO_EOF;
	}

	rc = close(stream->fd);
	if (rc < 0) {
		free(stream);
		return SO_EOF;
	}
	free(stream);
	return 0;
}

int so_fgetc(SO_FILE *stream)
{
	if (stream->bytesRead <= stream->cursor) {
		stream->bytesRead = read(stream->fd, stream->buffer, BUFFER_SIZE);

		if (stream->bytesRead < 0) {
			stream->error = 1;
			stream->bytesRead = 0;
			return SO_EOF;
		}

		if (stream->bytesRead == 0) {
			stream->endOfFile = 1;
			return SO_EOF;

		}

		stream->isWritting = 0;
		stream->cursor = 0;
		stream->error = 0;
	}
	stream->cursorFile++;
	return (int)(stream->buffer[stream->cursor++]);
}


int so_fputc(int c, SO_FILE *stream)
{
	int flush_id;

	if (stream->bytesRead >= BUFFER_SIZE) {
		flush_id = so_fflush(stream);
		if (flush_id == -1) {
			stream->error = 1;
			return -1;
		}
		stream->error = 0;
	}
	stream->cursorFile++;
	stream->buffer[stream->bytesRead++] = c;
	stream->isWritting = 1;

	return c;
}


size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int cursor_ptr = 0;
	int nr_bytes = 0;
	int n = size * nmemb;

	while (cursor_ptr < n) {
		if (stream->bytesRead <= stream->cursor) {
			stream->bytesRead = read(stream->fd, stream->buffer, BUFFER_SIZE);

			if (stream->bytesRead < 0) {
				stream->error = 1;
				stream->bytesRead = 0;
				return 0;
			}

			if (stream->bytesRead == 0) {
				stream->endOfFile = 1;
				stream->bytesRead = 0;
				break;
			}

			stream->error = 0;
			stream->isWritting = 0;
			stream->cursor = 0;
		}

		nr_bytes = stream->bytesRead - stream->cursor;
		if (nr_bytes > (n - cursor_ptr))
			nr_bytes = (n - cursor_ptr);

		memcpy(ptr + cursor_ptr, stream->buffer + stream->cursor, nr_bytes);
		stream->cursor += nr_bytes;
		stream->cursorFile += nr_bytes;
		cursor_ptr += nr_bytes;
	}

	return cursor_ptr / size;
}


size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) 
{
	int cursor_ptr = 0;
	int nr_bytes = 0;
	int n = size * nmemb;
	int bytesWritten;

	stream->isWritting = 1;

	while (cursor_ptr < n) {
		if (stream->bytesRead >= BUFFER_SIZE) {
			bytesWritten = xwrite(stream->fd, stream->buffer, stream->bytesRead);
			if (bytesWritten < 0) {
				stream->error = 1;
				return SO_EOF;
			}
			stream->error = 0;
			stream->bytesRead = 0;
			stream->cursor = 0;
		}

		if ((n - cursor_ptr) > (BUFFER_SIZE - stream->bytesRead))
			nr_bytes = BUFFER_SIZE - stream->bytesRead;
		else
			nr_bytes = (n - cursor_ptr);

		memcpy(stream->buffer + stream->bytesRead, ptr + cursor_ptr, nr_bytes);

		stream->bytesRead += nr_bytes;
		cursor_ptr += nr_bytes;
		stream->cursorFile += nr_bytes;
	}

	return cursor_ptr / size;
}


int so_fseek(SO_FILE *stream, long offset, int whence)
{
	int seek_result, bytesWritten;

	if (stream->bytesRead != 0 && stream->isWritting == 1) {
		bytesWritten = xwrite(stream->fd, stream->buffer, stream->bytesRead);
		if (bytesWritten < 0) {
			stream->error = 1;
			return SO_EOF;
		}
		stream->error = 0;
		stream->bytesRead = 0;
		stream->cursor = 0;
		stream->cursorFile += bytesWritten;
	}

	seek_result = lseek(stream->fd, offset, whence);

	if (seek_result != -1) {
		stream->cursor = 0;
		stream->cursor = 0;
		stream->bytesRead = 0;
		stream->cursorFile = seek_result;
		return 0;
	} else {
		return SO_EOF;
	}
}

long so_ftell(SO_FILE *stream)
{
	return stream->cursorFile;
}


int so_fflush(SO_FILE *stream)
{
	int bytesWritten;

	if (stream->isWritting == 1) {
		bytesWritten = xwrite(stream->fd, stream->buffer, stream->bytesRead);
		if (bytesWritten < 0) {
			stream->error = 1;
			return SO_EOF;
		}
		stream->error = 0;
		stream->bytesRead = 0;
		stream->cursor = 0;
		stream->cursorFile += bytesWritten;
	}
	return 0;
}

int so_fileno(SO_FILE *stream)
{
	return stream->fd;
}


int so_feof(SO_FILE *stream)
{
	return stream->endOfFile;
}


int so_ferror(SO_FILE *stream)
{
	return stream->error;
}


SO_FILE *so_popen(const char *command, const char *type)
{
	return NULL;
}


int so_pclose(SO_FILE *stream)
{
	return -1;
}
