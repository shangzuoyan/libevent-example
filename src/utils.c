#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "utils.h"

/* domain regular expression */
const char *rx_domain = "^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\\-]*[A-Za-z0-9])$";
/* ipv4 regular expression */
const char *rx_ipv4 = "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";
/* endpoint regular expression */
const char *rx_endpoint = "^(ipc|tcp)://.*$";

int check_regex(const char *er, const char *txt)
{
    regex_t regex;
    int ret;

    if (txt == NULL || !strlen(txt)) {
        return -1;
    }
    regcomp(&regex, er, REG_EXTENDED|REG_NOSUB);
    ret = regexec(&regex, txt,  0, NULL, 0);
    regfree(&regex);

    return ((ret == 0) ? 0 : -1);
}

int check_valid_domain(const char *domain)
{
    return check_regex(rx_domain, domain);
}

int check_valid_ipv4(const char *ipv4)
{
    return check_regex(rx_ipv4, ipv4);
}

int check_valid_endpoint(const char *endpoint)
{
    return check_regex(rx_endpoint, endpoint);
}

