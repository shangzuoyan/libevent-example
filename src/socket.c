/* For sockaddr_in */
#include <netinet/in.h>
/* For socket functions */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* For fcntl */
#include <fcntl.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "socket.h"
#include "utils.h"

void read_callback(struct bufferevent *bev, void *ctx)
{
    evutil_socket_t fd;
    struct evbuffer * input;
    socket_t * conn = (socket_t *)ctx;

    unsigned char bf[SOCKET_IOBUF_SIZE];
    int len;

    fd = bufferevent_getfd(bev);
    input = bufferevent_get_input(bev);

    /** clear recvbf */
    memset(bf, 0x00, sizeof(bf));
    /** read data */
    while ((len = evbuffer_remove(input, bf, sizeof(bf))) > 0) {
        if (conn->cb.on_read) {
            /* execute read callback */
            conn->cb.on_read(fd, bf, len);
        }
    }
}

void event_callback(struct bufferevent *bev, short events, void *ctx)
{
    socket_t * conn = (socket_t *)ctx;
    int fd = bufferevent_getfd(bev);
    /**
      *  connection has been closed, do any clean up here OR
      *  must be a timeout event handle, handle it
      */
    if ((events & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))) {
        if (conn->cb.on_disconnect) {
            /* execute disconnect callback */
            conn->cb.on_disconnect(fd, events);
        }
        close(fd);
        bufferevent_free(bev);
    } else if ((conn->mode = SOCKET_MODE_CLIENT) && (events & BEV_EVENT_CONNECTED)) {
        /* We're connected to 127.0.0.1:8080.
         * Ordinarily we'd do something here, like start reading or writing. */
        if (conn->cb.on_connect) {
            conn->cb.on_connect(fd);
        }
    }
}

/* accept callback */
static void do_accept(evutil_socket_t fds, short event, void *ctx)
{
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd;

    socket_t * conn = (socket_t *)ctx;

    UNUSED_ARG(event);

#if (SOCKET_TIMEOUT_ENABLED >= 1)
    struct timeval tv;

    tv.tv_sec = SOCKET_RECV_TIMEOUT;
    tv.tv_usec = 0;
#endif	/* (SERVER_TIMEOUT_ENABLED >= 1) */

    fd = accept(fds, (struct sockaddr*)&ss, &slen);
    if (fd < 0) {
        fprintf(stderr, "accept error: [%s]\n", strerror(errno));
        exit(1);
    } else if (fd > FD_SETSIZE) {
        close(fd);
    } else {
        evutil_make_socket_nonblocking(fd);

        /* client connected */
        conn->bev = bufferevent_socket_new(conn->base, fd, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(conn->bev,
                          read_callback,
                          NULL,
                          event_callback, conn);

        bufferevent_setwatermark(conn->bev, EV_READ, 0, SOCKET_IOBUF_SIZE);
        bufferevent_enable(conn->bev, EV_READ);

#if (SOCKET_TIMEOUT_ENABLED >= 1)
        /* set client timeout (2 minutes) */
        bufferevent_set_timeouts(conn->bev, &tv, NULL);
#endif	/* (SERVER_TIMEOUT_ENABLED >= 1) */

        if (conn->cb.on_connect) {
            /* execute connect callback */
            conn->cb.on_connect(fd);
        }
    }
}

static int socket_copy(socket_t *conn, const char *endpoint, socket_callback_t *cb)
{
    /* clear all connection struct */
    if (!conn || !endpoint || !strlen(endpoint) || !cb) {
        return -1;
    }
    memset(conn, 0x00, sizeof(socket_t));

    /* copy param */
    memcpy(&conn->cb, cb, sizeof(socket_callback_t));

    /* decoding endpoint and save it on data */
    return get_endpoint(&conn->data, endpoint);
}

int socket_bind(socket_t *conn, const char *endpoint, socket_callback_t *cb)
{
    struct sockaddr *addr = NULL;
    int length = 0;

    if (socket_copy(conn, endpoint, cb) < 0) {
        return -1;
    }

    /* Create and return a new event_base to use with the rest of Libevent. */
    conn->base = event_base_new();
    if (!conn->base) {
        return -1;
    }
    conn->mode = SOCKET_MODE_SERVER;

    switch(conn->data.type) {
    case EVT_TYPE_IPC:
        unlink(conn->data.host);

        conn->sun.sun_family = AF_UNIX;
        strncpy(conn->sun.sun_path, conn->data.host, (sizeof(conn->sun.sun_path) - 1));
        conn->listener = socket(AF_UNIX, SOCK_STREAM, 0);

        /** set sockaddr and length to bind */
        addr = (struct sockaddr *)&conn->sun;
        length = (strlen(conn->sun.sun_path) + sizeof(conn->sun.sun_family));
        break;

    case EVT_TYPE_TCP:
        conn->sin.sin_family = AF_INET;
        conn->sin.sin_addr.s_addr = 0;
        conn->sin.sin_port = htons(conn->data.port);
        conn->listener = socket(AF_INET, SOCK_STREAM, 0);

        /** set sockaddr and length to bind */
        addr = (struct sockaddr *)&conn->sin;
        length = sizeof(conn->sun);
        break;
    }

    evutil_make_socket_nonblocking(conn->listener);

    {
        int one = 1;
        setsockopt(conn->listener,
                   SOL_SOCKET, SO_REUSEADDR,
                   &one, sizeof(one));
    }

    if (bind(conn->listener, addr, length) < 0) {
        fprintf(stderr, "bind error: [%s]\n", strerror(errno));
        return -1;
    }

    if (listen(conn->listener, SOCKET_MAX_ACCEPT) < 0) {
        fprintf(stderr, "listen error: [%s]\n", strerror(errno));
        return -1;
    }
    conn->listener_event = event_new(conn->base,
                                     conn->listener,
                                     EV_READ|EV_PERSIST,
                                     do_accept,
                                     conn);
    event_add(conn->listener_event, NULL);

    /* lock */
    event_base_dispatch(conn->base);

    return 0;
}

int socket_close(socket_t *conn)
{
    if(!conn) {
        return -1;
    }
    if (conn->mode == SOCKET_MODE_SERVER) {
        int ret;
        if (conn->listener_event) {
            event_del(conn->listener_event);
            if ((ret = close(conn->fd)) != 0) {
                fprintf(stderr, "close fd %d error: [%s]\n",
                        conn->fd, strerror(errno));
            }
            return ret;
        }
    } else if ((conn->mode == SOCKET_MODE_CLIENT) && (!socket_is_connected(conn))) {
        bufferevent_free(conn->bev);
        conn->bev = NULL;
    }
    return -1;
}

int socket_send(socket_t * conn, const void *bf, int len)
{
    struct evbuffer *output;

    if (!conn || !conn->bev || !bf) {
        return -1;
    }
    output = bufferevent_get_output(conn->bev);
    return evbuffer_add(output, bf, len);
}

int socket_write(int fd, const void *bf, int len)
{
    if ((fd < 0) || !bf || (len < 0)) {
        return -1;
    }
    if (write(fd, bf, len) < 0) {
        fprintf(stderr, "write error: [%s]\n", strerror(errno));
        return -1;
    }
    return 0;
}

/**
 *	client methods
 */

int socket_connect(socket_t *conn, const char *endpoint, socket_callback_t *cb)
{
    struct sockaddr *addr = NULL;
    int length = 0;

    if (!conn || !endpoint || !strlen(endpoint)) {
        return -1;
    }
    /* clear connection info */
    memset(conn, 0x00, sizeof(socket_t));

    /* copy param */
    memcpy(&conn->cb, cb, sizeof(socket_callback_t));
    conn->mode = SOCKET_MODE_CLIENT;

    if (get_endpoint(&conn->data, endpoint) < 0) {
        return -1;
    }

    conn->base = event_base_new();
    if (!conn->base) {
        return -1;
    }

    switch(conn->data.type) {
    case EVT_TYPE_IPC:
        conn->sun.sun_family = AF_UNIX;
        strncpy(conn->sun.sun_path, conn->data.host, (sizeof(conn->sun.sun_path) - 1));

        /** set sockaddr and length to connect */
        addr = (struct sockaddr *)&conn->sun;
        length = (strlen(conn->sun.sun_path) + sizeof(conn->sun.sun_family));
        break;

    case EVT_TYPE_TCP:
        conn->sin.sin_family = AF_INET;
        conn->sin.sin_port = htons(conn->data.port);
        inet_aton(conn->data.host, &conn->sin.sin_addr);

        /** set sockaddr and length to connect */
        addr = (struct sockaddr *)&conn->sin;
        length = sizeof(conn->sun);
        break;
    }
    conn->bev = bufferevent_socket_new(conn->base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(conn->bev,
                      read_callback,
                      NULL,
                      event_callback, conn);

    bufferevent_setwatermark(conn->bev, EV_READ, 0, SOCKET_MAX_LINE);
    bufferevent_enable(conn->bev, EV_READ);

    if (bufferevent_socket_connect(conn->bev, addr, length) < 0) {
        bufferevent_free(conn->bev);
        return -1;
    }

    event_base_dispatch(conn->base);
    return 0;
}

int socket_is_connected(socket_t * conn)
{
    return ((!conn || !conn->bev) ? -1 : 0);
}

