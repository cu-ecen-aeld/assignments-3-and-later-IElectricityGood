#include <syslog.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]){

    /*
     * NULL -> ident is NULL -> default id string is argv[0] (program name)
     * 0 -> no options flags set (such as LOG_PID, LOG_NDELAY, LOG_PERROR or LOG_CONS)
     */
    openlog(NULL, 0, LOG_USER);

    if (argc != 3){
        syslog(LOG_ERR, "Invalid number of arguments provided: %d (should be 2)", argc-1);
        return 1;
    }

    char* writefile = argv[1];
    char* writestr = argv[2];

    int fd;
    fd = open(writefile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1){ // error
        syslog(LOG_ERR, "Error opening file for write: %s", writefile);
        return 1;
    }

    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);
    
    ssize_t nwrite1, nwrite2;
    nwrite1 = write(fd, writestr, strlen(writestr));
    nwrite2 = write(fd, "\n", 1);
    if (nwrite1 == -1 || nwrite2 == -1){ // error
        syslog(LOG_ERR, "Error writing to file for write: %s", writefile);
        return 1;
    }

    int rc_sync;
    rc_sync = fsync(fd);
    if (rc_sync == -1){ // error
        syslog(LOG_ERR, "Error flushing file: %s", writefile);
        return 1;
    }

    int rc_close;
    rc_close = close(fd);
    if (rc_close == -1){ // error
        syslog(LOG_ERR, "Error closing file: %s", writefile);
        return 1;
    }

    return 0;
}
