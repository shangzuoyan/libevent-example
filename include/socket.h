#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

/**
 *	@file socket.h
 *	@brief TCP or IPC socket server / client
 */

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "endpoint.h"

#define SOCKET_IOBUF_SIZE		2048
#define SOCKET_MAX_ACCEPT		1024
#define SOCKET_RECV_TIMEOUT		120 /* seconds (2 minutes) */
#define SOCKET_MAX_LINE			16384

typedef enum {
    SOCKET_MODE_SERVER = 0x00,
    SOCKET_MODE_CLIENT,
} socket_mode_t;

typedef struct {
    /** This callback is called when client connect on server */
    void (*on_connect)(int fd);

    /** This callback is called when client disconnected */
    void (*on_disconnect)(int fd, short events);

    /** This callback is called when socket received some data */
    void (*on_read)(int fd, void *buffer, int length);
} socket_callback_t;

typedef struct {
    /** connection file description */
    int fd;

    /** event data info */
    evt_data_t data;

    /** Create and return a new event_base to use with the rest of Libevent. */
    struct event_base * base;

    /** used in TCP transport */
    struct sockaddr_in sin;

    /** used in Inter-Process communication */
    struct sockaddr_un sun;

    /** listening event */
    evutil_socket_t listener;
    struct event *listener_event;

    /** callback list */
    socket_callback_t cb;

    /** event buffer */
    struct bufferevent *bev;

    /** socket mode (server or client) */
    socket_mode_t mode;
} socket_t;

/**
 *  Close current server.
 *
 *  @param [in]  conn       Socket connection struct
 *
 *  @return     Success, zero is returned.
 *  @return     On error, -1 is returned.
 */
int socket_close(socket_t *conn);

/**
 *	Accept connection on a socket.
 *
 **	@param [in]  conn       Socket connection struct.
 *	@param [in]  endpoint   String with endpoint connection description.
 *	@param [in]  cb         Callback structure.
 *
 *  @return     Success, zero is returned.
 *  @return     On error, -1 is returned.
 */
int socket_bind(socket_t *conn,
                const char *endpoint,
                socket_callback_t *cb);
/**
 *  Send buffer via socket using connection(bev)
 *
 *  @param [in] conn	Socket connection struct
 *  @param [in] bf  	Buffer
 *  @param [in] len 	Buffer length
 *
 *  @return     Success, zero is returned.
 *  @return     On error, -1 is returned.
 */
int socket_send(socket_t * conn, const void *bf, int len);

/**
 *  Write buffer via socket.
 *
 *  @param [in] fd  Socket file descriptor
 *  @param [in] bf  Buffer
 *  @param [in] len Buffer length
 *
 *  @return     Success, zero is returned.
 *  @return     On error, -1 is returned.
 */
int socket_write(int fd, const void *bf, int len);

/**
 *  Connect to a remote server (TCP) or a inter-process file (IPC)
 *
 *  @param [in]  conn       Socket connection struct.
 *	@param [in]  endpoint   String with endpoint connection description.
 *	@param [in]  cb         Callback structure.
 *
 *  @return     Success, zero is returned.
 *  @return     On error, -1 is returned.
 */
int socket_connect(socket_t *conn,
                   const char *endpoint,
                   socket_callback_t *cb);

/**
 *  Indicate if socket is connected to a remote server
 *
 *  @param [in] conn    Socket connection struct.
 *
 *  @return     Success, zero is returned.
 *  @return     On error, -1 is returned.
 */
int socket_is_connected(socket_t * conn);

#endif	/* SERVER_H_INCLUDED */
