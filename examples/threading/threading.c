#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
// #define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("[threading LOG]: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("[threading ERROR]: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct thread_data* thread_func_args = (struct thread_data*) thread_param;

    bool thread_complete_success = true;
    pthread_mutex_t* mutex = thread_func_args->mutex;

    DEBUG_LOG("Entered thread %li", (unsigned long int) thread_func_args->thread);

    usleep(thread_func_args->wait_to_obtain_ms * 1000);
    int rc;
    
    // DEBUG_LOG("Mutex time for thread %li", (unsigned long int) thread_func_args->thread);
    rc = pthread_mutex_lock(mutex);
    if (rc != 0){
        ERROR_LOG("Attempt to obtain mutex [ %li ] failed with %d", (unsigned long int) mutex, rc);
        thread_complete_success = false;
    }
    else{
        DEBUG_LOG("Successfully obtained mutex [ %li ]", (unsigned long int) mutex);
    }

    usleep(thread_func_args->wait_to_release_ms * 1000);

    rc = pthread_mutex_unlock(mutex);
    if (rc != 0){
        ERROR_LOG("Attempt to unlock mutex [ %li ] failed with %d", (unsigned long int) mutex, rc);
        thread_complete_success = false;
    }
    else{
        DEBUG_LOG("Successfully unlocked mutex [ %li ]", (unsigned long int) mutex);
    }

    thread_func_args->thread_complete_success = thread_complete_success;
    DEBUG_LOG("Exiting thread %li", (unsigned long int) thread_func_args->thread);
    return thread_func_args;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    
    struct thread_data *thread_param = malloc(sizeof(struct thread_data));
    thread_param->thread_complete_success = false;
    thread_param->mutex = mutex;
    thread_param->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_param->wait_to_release_ms = wait_to_release_ms;

    int rc = pthread_create(thread, NULL, threadfunc, (void*) thread_param);
    if (rc == 0){
        thread_param->thread = thread;
        DEBUG_LOG("thread created successfully, thread id = %li", (unsigned long int) thread);
        return true;
    }
    else{
        ERROR_LOG("thread creation failed");
        return false;
    }
}

