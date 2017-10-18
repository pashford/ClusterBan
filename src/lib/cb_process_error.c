/*
 * vim: sw=4 ai
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "cb_lib.h"

#define ERROR_BUF_SIZE 256

int cb_min_log_level = 0;

/*
 * Set the minimum priority for logging.
 */
void cb_set_log_level(int level) {
    if(level >= 0)
	cb_min_log_level = level;
    else
	cb_min_log_level = 0;
    return;
}

/*
 * Send an error to Syslog, if it's a high enough priority.
 */
void cb_process_error(int log_priority, char * message, int errsv) {
    char error_buf[ERROR_BUF_SIZE];
    int rtn;

    if(log_priority >= cb_min_log_level) {
	if(errsv != 0) {
	    rtn = strerror_r(errsv, error_buf, ERROR_BUF_SIZE);
	    if(rtn != 0) {
	    	cb_process_error(CB_ERROR_FATAL, "Error converting errno", errno);
		exit(-1);
	    }
	    syslog(log_priority, "%s - %s", message, error_buf);
	} else {
	    syslog(log_priority, "%s", message);
	}
    }
    return;
}
