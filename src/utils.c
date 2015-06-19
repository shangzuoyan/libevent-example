#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "utils.h"

int check_regex(const char *er, const char *txt)
{
    regex_t regex;
    int ret;

    if (!txt || !strlen(txt)) {
        return -1;
    }
    regcomp(&regex, er, REG_EXTENDED|REG_NOSUB);
    ret = regexec(&regex, txt,  0, NULL, 0);
    regfree(&regex);

    return ((ret != 0) ? -1 : 0);
}

int check_valid_domain(const char *domain)
{
    const char *domain_er = "^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\\-]*[A-Za-z0-9])$";
    return check_regex(domain_er, domain);
}

int check_valid_ipv4(const char *ipv4)
{
    const char *ipv4_er = "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";
    return check_regex(ipv4_er, ipv4);
}

int check_valid_endpoint(const char *endpoint)
{
    const char *endpoint_er = "^(ipc|tcp)://.*$";
    return check_regex(endpoint_er, endpoint);
}

