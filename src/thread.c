#include <thread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int thread_start(thread_t *tc,
                 thread_detach_type_t type,
                 thread_routine_t * routine, void *arg)
{
    int ret;
    pthread_t *thread;
    pthread_attr_t *attr;

    if (tc == NULL || routine == NULL) {
        return -1;
    }
    thread = &tc->thread;
    attr = &tc->attr;

    /** set thread attributes */
    pthread_attr_init(&tc->attr);
    ret = pthread_create(thread, attr, routine, arg);
    if (ret != 0) {
        fprintf(stderr, "pthread create error:[%s]\n", strerror(errno));
        return -1;
    }

    switch(type) {
    case THREAD_DETACHED:
        if (pthread_detach(tc->thread) < 0) {
            fprintf(stderr, "detach error:[%s]\n", strerror(errno));
            return -1;
        }
        break;
    case THREAD_JOINABLE:
        if (pthread_join(tc->thread, NULL) < 0) {
            fprintf(stderr, "join error:[%s]\n", strerror(errno));
            return -1;
        }
        break;
    default:
        return -1;
    }
    return 0;
}

void thread_stop(thread_t *tc)
{
    pthread_cancel(tc->thread);
}

