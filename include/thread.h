#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED

/**
 *	@file pthread.h
 *	@brief Pthread API
 */

#include <pthread.h>

typedef enum {
    THREAD_DETACHED = 0,
    THREAD_JOINABLE,
} thread_detach_type_t;

typedef struct {
    pthread_t thread; /* pthread descriptor */
    pthread_attr_t attr; /* pthread attribute (callback function) */
} thread_t;

typedef void *(thread_routine_t)(void *);

/**
 *	Initialize thread attribute *ATTR with default attributes and
 *	create a new thread and execute routine
 *
 *	@param [in] tc      Thread config
 *	@param [in] state   Thread detach type (DETACHED or JOINABLE)
 *  @param [in] routine Thread routine called after start
 *  @param [in] arg     Argument of thread routine
 */
int thread_start(thread_t *tc,
                 thread_detach_type_t type,
                 thread_routine_t * routine, void *arg);
/**
 *	Cancel THREAD immediately or at the next possibility.
 *
 *  @param [in] tc      Thread config
 */
void thread_stop(thread_t *tc);

#endif	/* THREAD_H_INCLUDED */
