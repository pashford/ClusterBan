/*
 * vim: sw=4 ai
 */

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

/*
 * ear.c
 *
 * This routine listens for connection attempts.  When a new connection
 * is made, build a structure with the IP address, port and time, and
 * put the address of the structure into a message queue, close the
 * connection and listen again.
 */

#include "common.h"

/* This is used on the "listen" call. */
#define LISTEN_QUEUE_DEPTH 64

/* TODO
 * Handle SIGPIPE errors
 */

/*
 * With up to 500 total ears, we can't afford to waste the space of having
 * IPV4 and an IPV6 socket structures in each.  Therefore, we have different
 * routines for IPV4 and IPV6.  This also simplifies the process of porting
 * the code to BSD.
 */
static void * setup_ear4(void *p_port) {
    extern struct parameters global_parameters;
    struct sockaddr_in socket_struct;
    struct sockaddr_in last_connection;
    struct cb_event *message;
    struct timespec timer_sleep, ddos_sleep;
    time_t cur_time, last_time, ddos_time;
    int port;
    int sock_fd;
    int addr_len = 4;
    int rtn;

    port = *((int *) p_port);
    timer_sleep.tv_sec = 0;
    timer_sleep.tv_nsec = 100000000;
    ddos_sleep.tv_sec = 10;
    ddos_sleep.tv_nsec = 0;

    sock_fd = socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, IPPROTO_TCP);
    if(sock_fd == -1) {
    	process_error(ERROR_MAJOR, "Error creating IPV4 socket", errno);
	rtn = -1;
	pthread_exit(&rtn);
    }

    socket_struct.sin_family = AF_INET;
    memcpy(&socket_struct.sin_addr.s_addr, global_parameters.addr_ipv4, addr_len);
    rtn = bind(sock_fd, (struct sockaddr*) &socket_struct, sizeof(socket_struct));
    if(rtn) {
	process_error(ERROR_MAJOR, "Error binding on IPV4 socket", errno);
	rtn = -1;
	pthread_exit(&rtn);
    }

    rtn = listen(sock_fd, LISTEN_QUEUE_DEPTH);
    if(rtn) {
	process_error(ERROR_MAJOR, "Error listening on IPV4 socket", errno);
	rtn = -1;
	pthread_exit(&rtn);
    }

    last_time = 0;
    memset(&last_connection, 0, sizeof(last_connection));
    ddos_time = 0;

    for(;;) {
    	rtn = accept(sock_fd, (struct sockaddr*) &socket_struct, &addr_len);
	if(rtn = -1)
	    continue;
	cur_time = time(NULL);

	/*
	 * Do the data reduction checks.  This reduces events placed on the
	 * pipe when there's a directed attack or a complex scan.
	 */
	if(!memcmp(&last_connection.sin_addr.s_addr, &socket_struct.sin_addr.s_addr, addr_len)) {
	    if((cur_time - last_time) < 60) {
	    	continue;
	    }
	}

	/* Put the information on the queue */
	message = malloc(sizeof(struct cb_event));
	message->event_type = CB_EVENT_CON_IPV4;
	message->event_port = port;
	message->event_time = cur_time;
	memcpy(message->event_address, &socket_struct.sin_addr.s_addr, addr_len);
	rtn = write(global_parameters.message_queue[1], message, sizeof(void *));
	if(rtn != sizeof(void *)) {
	    free(message);
	    /* Don't swamp the Syslog server with DDOS messages */
	    if((cur_time - global_parameters.ddos_message_time) > DDOS_MESSAGE_INTERVAL) {
		process_error(ERROR_MINOR, "Event message queue full - Possible DDOS");
		global_parameters.ddos_message_time = cur_time;
	    }
	    /* Give the message queue some time to drain */
	    if((cur_time - ddos_time) > 10) {
		nanosleep(&timer_sleep, NULL);
	    } else {
		/* If we're in a DDOS on this port, wait a bit */
		nanosleep(&ddos_sleep, NULL);
	    }
	    ddos_time = cur_time;
	}
	close(rtn);
    }
}

static void * setup_ear6(void *p_port) {
    extern struct parameters global_parameters;
    struct sockaddr_in6 socket_struct;
    struct sockaddr_in6 last_connection;
    struct cb_event *message;
    struct timespec timer_sleep, ddos_sleep;
    time_t cur_time, last_time, ddos_time;
    int port;
    int sock_fd;
    int addr_len = 16;
    int rtn;
    int sock_val = 1;

    port = *((int *) p_port);
    timer_sleep.tv_sec = 0;
    timer_sleep.tv_nsec = 100000000;
    ddos_sleep.tv_sec = 10;
    ddos_sleep.tv_nsec = 0;

    sock_fd = socket(AF_INET6, SOCK_STREAM|SOCK_CLOEXEC, IPPROTO_TCP);
    if(sock_fd == -1) {
    	process_error(ERROR_MAJOR, "Error creating IPV6 socket", errno);
	rtn = -1;
	pthread_exit(&rtn);
    }

    /* Set the socket to IPV6-only */
    rtn = setsockopt(sock_fd, SOL_SOCKET, IPV6_V6ONLY, &sock_val, sizeof(int));
    if(rtn == -1) {
	process_error(ERROR_MAJOR, "Error limiting IPV6 socket", errno);
	rtn = -1;
	pthread_exit(&rtn);
    }

    socket_struct.sin6_family = AF_INET6;
    memcpy(socket_struct.sin6_addr.s6_addr, global_parameters.addr_ipv6, addr_len);
    rtn = bind(sock_fd, (struct sockaddr*) &socket_struct, sizeof(socket_struct));
    if(rtn) {
	process_error(ERROR_MAJOR, "Error binding on IPV6 socket", errno);
	rtn = -1;
	pthread_exit(&rtn);
    }

    rtn = listen(sock_fd, LISTEN_QUEUE_DEPTH);
    if(rtn) {
	process_error(ERROR_MAJOR, "Error listening on IPV6 socket", errno);
	rtn = -1;
	pthread_exit(&rtn);
    }

    last_time = 0;
    memset(&last_connection, 0, sizeof(last_connection));
    ddos_time = 0;

    for(;;) {
    	rtn = accept(sock_fd, (struct sockaddr*) &socket_struct, &addr_len);
	if(rtn = -1)
	    continue;
	cur_time = time(NULL);

	/*
	 * Do the data reduction checks.  This reduces events placed on the
	 * pipe when there's a directed attack or a complex scan.
	 */
	if(!memcmp(&last_connection.sin6_addr.s6_addr, &socket_struct.sin6_addr.s6_addr, addr_len)) {
	    if((cur_time - last_time) < 60) {
	    	continue;
	    }
	}

	/* Put the information on the queue */
	message = malloc(sizeof(struct cb_event));
	message->event_type = CB_EVENT_CON_IPV6;
	message->event_port = port;
	message->event_time = cur_time;
	memcpy(message->event_address, &socket_struct.sin6_addr.s6_addr, addr_len);
	rtn = write(global_parameters.message_queue[1], message, sizeof(void *));
	if(rtn != sizeof(void *)) {
	    free(message);
	    /* Don't swamp the Syslog server with DDOS messages */
	    if((cur_time - global_parameters.ddos_message_time) > DDOS_MESSAGE_INTERVAL) {
		process_error(ERROR_MINOR, "Event message queue full - Possible DDOS");
		global_parameters.ddos_message_time = cur_time;
	    }
	    /* Give the message queue some time to drain */
	    if((cur_time - ddos_time) > 10) {
		nanosleep(&timer_sleep, NULL);
	    } else {
		/* If we're in a DDOS on this port, wait a bit */
		nanosleep(&ddos_sleep, NULL);
	    }
	    ddos_time = cur_time;
	}
	close(rtn);
    }
}

/* Create an Ear thread for IPV4, and, optionally for IPV6 */
void create_ear(int port) {
    extern struct parameters global_parameters;
    extern struct threads *thread_root, *thread_tail;
    int rtn;

    /* The alloc_thread routine isn't thread-safe.  Only call it from the main thread. */
    rtn = alloc_thread(port, THREAD_IPV4);
    if(! rtn) {
	rtn = pthread_create(&thread_root->data, NULL, &setup_ear4, &port);
	if(rtn) {
	    process_error(ERROR_MAJOR, "Error creating IPV4 scan detector thread", errno);
	}
    }

    if(global_parameters.accept_ipv6) {
	/* The alloc_thread routine isn't thread-safe.  Only call it from the main thread. */
	rtn = alloc_thread(port, THREAD_IPV6);
	if(! rtn) {
	    rtn = pthread_create(&thread_root->data, NULL, &setup_ear6, &port);
	    if(rtn) {
		process_error(ERROR_MAJOR, "Error creating IPV6 scan detector thread", errno);
	    }
	}
    }
    return;
}
