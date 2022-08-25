#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

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

    FILE* fp;
    fp = fopen(writefile, "w");
    if (fp == NULL){
        syslog(LOG_ERR, "Error opening file for write: %s", writefile);
        return 1;
    }

    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);
    int rc = fprintf(fp, "%s\n", writestr);
    if (rc < 0){
        syslog(LOG_ERR, "Error writing to file for write: %s", writefile);
        return 1;
    }

    fclose(fp);
    return 0;
}
