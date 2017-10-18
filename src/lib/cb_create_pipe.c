/*
 * vim: sw=4 ai
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "cb_lib.h"

struct parameters global_parameters;

/* Set the pipe size to 64KB */
#define PIPE_SIZE 64*1024
#define MESSAGE_BUFFER 1024

/*
 * Create a pipe and set it up for use as a message queue.
 *
 * Unnamed pipes are used for communication between threads of a program.
 * The address of a data structure is placed onto the pipe.  When the
 * address is read, the data in the structure is processed.  Upon
 * completion, it is the responsibility of the reader to free the
 * structure.
 *
 * The pipe is configured to block on reads, but not on writes.  If the
 * pipe is full, the write will return immediately with an error.  This
 * error must be processed and the data structure deallocated.
 *
 * Suggested use is to create all the needed pipes in the main thread
 * before the child threads are created.  Then, create the threads that
 * reads the pipes.  Finally, create the threads that write to the pipes.
 */
void cb_create_pipe(int *pipe_fd, char *reason) {
    char message[MESSAGE_BUFFER];
    int errsv;
    int rtn;

    /* Create the pipe for use as a message queue */
    rtn = pipe2(pipe_fd, O_CLOEXEC|O_DIRECT);
    if(rtn == -1) {
	errsv = errno;
	snprintf(message, MESSAGE_BUFFER, "Error creating pipe for %s", reason);
	cb_process_error(CB_ERROR_FATAL, message, errsv);
	exit(-1);
    }
    
    /* Increase the pipe buffer so that it's less likely to overflow */
    rtn = fcntl(pipe_fd[0], F_SETPIPE_SZ, PIPE_SIZE);
    if(rtn == -1) {
	errsv = errno;
	snprintf(message, MESSAGE_BUFFER, "Error configuring pipe size for %s", reason);
	cb_process_error(CB_ERROR_FATAL, message, errsv);
	exit(-1);
    }

    /* Writes shouldn't block - Dont do this for the read FD, as the read should block */
    rtn = fcntl(pipe_fd[1], F_SETFL, O_NONBLOCK);
    if(rtn == -1) {
	errsv = errno;
	snprintf(message, MESSAGE_BUFFER, "Error configuring pipe I/O for %s", reason);
	cb_process_error(CB_ERROR_FATAL, message, errsv);
	exit(-1);
    }
    return;
}
