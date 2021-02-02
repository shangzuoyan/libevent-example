#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "endpoint.h"
#include "utils.h"

int get_endpoint(evt_data_t * data, const char *endpoint)
{
    char *s;
    const char * transport_type[] = {
        "ipc://", "tcp://", NULL,
    };
    int i = 0;
    const char *pstx, *petx;
    int length;

    if (data == NULL || endpoint == NULL || strlen(endpoint) == 0) {
        return -1;
    }

    if (check_valid_endpoint(endpoint) != 0) {
        fprintf(stderr, "invalid endpoint (%s)\n", endpoint);
        return -1;
    }

    while(transport_type[i]) {
        if ((s = strstr(endpoint, transport_type[i++])) != NULL) {
            pstx = strstr(endpoint, "//");
            if (pstx) {
                data->type = (evt_type_t)(i - 1);
                pstx += strlen("//");

                switch(data->type) {
                case EVT_TYPE_IPC:
                    if (strlen(pstx) != 0) {
                        strncpy(data->host, pstx, sizeof(data->host));
                        return 0;
                    }
                    return -1;

                case EVT_TYPE_TCP:
                    petx = strchr(pstx, ':');
                    if (petx) {
                        petx = (petx - 1);
                        length = ((petx - pstx) > (int)sizeof(data->host)) ?
                                 (int)sizeof(data->host) :
                                 (int)(petx - pstx + 1);
                        strncpy(data->host, pstx, length);
                        if (strcmp(data->host, "*") != 0) {
                            if ((check_valid_ipv4(data->host) != 0)
                                && (check_valid_domain(data->host) != 0)) {
                                /* invalid host */
                                fprintf(stderr, "invalid host (%s)\n", data->host);
                                return -1;
                            }
                        }
                        data->port = atoi(petx + 2);
                        return 0;
                    }
                    break;
                }
            }
        }
    }
    return -1;
}


