#ifndef ENDPOINT_H_INCLUDED
#define ENDPOINT_H_INCLUDED

#include <limits.h>

/**
 *  IPC:
 *  Assign the pathname "/tmp/feeds/0"
 *      rc = socket_bind(socket, "ipc:///tmp/feeds/0"); assert (rc == 0);
 *
 *  Connect to the pathname "/tmp/feeds/0"
 *      rc = socket_connect(socket, "ipc:///tmp/feeds/0"); assert (rc == 0);
 *
 *  TPC:
 *  Assigning a local address to a socket
 *      TCP port 5555 on all available interfaces
 *          rc = socket_bind(socket, "tcp:// *:5555");
 *          assert (rc == 0);
 *
 *      TCP port 5555 on the local loop-back interface on all platforms
 *          rc = socket_bind(socket, "tcp://127.0.0.1:5555");
 *          assert (rc == 0);
 *
 *      TCP port 5555 on the first Ethernet network interface on Linux
 *          rc = socket_bind(socket, "tcp://eth0:5555"); assert (rc == 0);
 *
 *  Connecting a socket
 *      Connecting using an IP address
 *          rc = socket_connect(socket, "tcp://192.168.1.1:5555");
 *          assert (rc == 0);
 */

typedef enum {
    EVT_TYPE_IPC = 0x00,    /**< Inter-Process communication transport */
    EVT_TYPE_TCP,           /**< Unicast transport using TCP */
} evt_type_t;

typedef struct {
    char host[HOST_NAME_MAX];
    unsigned int port;
    evt_type_t type;
} evt_data_t;

/**
 * Decode endpoint string and get data.
 *
 *  @param [out] data
 *  @param [in]  endpoint   Endpoint string (e.g.:"ipc:///tmp/feeds/0")
 *
 *  @return     Success, zero is returned.
 *  @return     On error, -1 is returned.
 */
int get_endpoint(evt_data_t * data, const char *endpoint);

#endif  /* ENDPOINT_H_INCLUDED */
