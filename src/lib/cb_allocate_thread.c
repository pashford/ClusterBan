/*
 * vim: sw=4 ai
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "cb_lib.h"

/*
 * Allocate a Thread structure and link it into the chain.
 *
 * This routine is NOT thread safe.  It should be called only from the
 * main thread.
 */
int alloc_thread(int port, char t_type) {
    extern struct threads *thread_root, *thread_tail;
    struct threads *old_root;

    old_root = thread_root;
    thread_root = (struct threads *) malloc(sizeof(struct threads));
    if(thread_root == NULL) {
    	thread_root = old_root;
	cb_process_error(CB_ERROR_MAJOR, "Error allocating thread structure", errno);
	return(1);
    }
    /* The new entry is placed at the head of the list */
    thread_root->next = old_root;
    thread_root->prev = NULL;
    thread_root->port = port;
    thread_root->type = t_type;
    old_root->prev = thread_root;
    if(old_root == NULL) {
    	thread_tail = thread_root;
    }
    return(0);
}
