#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "thread.h"
#include "socket.h"
#include "utils.h"

const char *endpoint = "tcp://127.0.0.1:20000";

void client_connect_cb(int fd)
{
    const char message[] = "ping";
    fprintf(stderr, "client::%s\n", message);
    socket_write(fd, message, strlen(message));
}

void client_read_cb(int fd, void *buffer, int length)
{
    const char message[] = "ping";

    if ((length > 0) && (buffer != NULL)) {
        if (strcasecmp("pong", buffer) == 0) {
            fprintf(stderr, "client::%s\n", message);
            socket_write(fd, message, strlen(message));
            sleep(1);
        }
    }
}

void client_disconnect_cb(int fd, short events)
{
    UNUSED_ARG(events);
    fprintf(stderr, "server::disconnected to %d\n", fd);
}

void server_connect_cb(int fd)
{
    // added client file descriptor
    fprintf(stderr, "server::connected to %d\n", fd);
}

void server_disconnect_cb(int fd, short events)
{
    UNUSED_ARG(events);
    fprintf(stderr, "server::disconnected to %d\n", fd);
}

void server_on_read_cb(int fd, void *buffer, int length)
{
    const char message[] = "pong";

    if ((length > 0) && (buffer != NULL)) {
        if (strcasecmp("ping", buffer) == 0) {
            fprintf(stderr, "server::%s\n", message);
            socket_write(fd, message, strlen(message));
        }
    }
}

void * server_thread_cb(void *arg)
{
    // socket listen & bind connection
    socket_t server;

    // socket callback
    socket_callback_t server_cb = {
        .on_connect = &server_connect_cb,
        .on_disconnect = &server_disconnect_cb,
        .on_read = &server_on_read_cb,
    };

    UNUSED_ARG(arg);

    while(1) {
        int ret = socket_bind(&server, endpoint, &server_cb);
        if (ret < 0) {
            fprintf(stderr, "socket listen error\n");
            goto on_exit;
        }
    }

on_exit:
    socket_close(&server);
    return NULL;
}


int main(void)
{
    int ret;
    thread_t thread;

    ret  = thread_start(&thread, THREAD_DETACHED,
                        (thread_routine_t *)server_thread_cb, NULL);
    if (ret < 0) {
        return -1;
    }

    // client connection
    socket_t client;
    socket_callback_t client_cb = {
        .on_connect = &client_connect_cb,
        .on_read =  &client_read_cb,
        .on_disconnect = &client_disconnect_cb,
    };
    sleep(1);

	fprintf(stderr, "try to connect on endpoint: %s\n", endpoint);
    /** client connection */
    if (socket_connect(&client, endpoint, &client_cb) < 0) {
        return -1;
    }
    // close client connection
    socket_close(&client);

    // server stop
    thread_stop(&thread);
    return 0;
}
