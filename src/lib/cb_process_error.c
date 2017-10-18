/*
 * vim: sw=4 ai
 */

#include <errno.h>
#include <string.h>

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
    char *error_msg;

    if(log_priority >= cb_min_log_level) {
	if(errsv != 0) {
	    error_msg = strerror_l(errsv, error_buf, ERROR_BUF_SIZE);
	    syslog(log_priority, "%s - %s", message, error_msg);
	} else
	    syslog(log_priority, "%s", message);
	}
    }
    return;
}
