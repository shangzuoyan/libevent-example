#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#ifndef UNUSED_ARG
#define UNUSED_ARG(arg)  (void)arg
#endif


/**
 * Function to check a string using a regular expression.
 *
 * @param [in] er   Regular Expression string
 * @param [in] txt  String to check
 *
 * @return     Success, zero is returned.
 * @return     On error, -1 is returned.
 */
int check_regex(const char *er, const char *txt);

/**
 *	Check if domain is valid using regular expression below:
 *			"^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$";.
 *
 * 	@param [in] domain  Domain string e.g.:(server.net)
 *
 *  @return     Success, zero is returned.
 *  @return     On error, -1 is returned.
 */
int check_valid_domain(const char *domain);

/**
 *	Check if ipv4 address is valid using regular expression below:
 *			"^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";.
 *
 *	@param [in] ipv4    IPV4 string (e.g.: 192.168.1.1)
 *
 *  @return     Success, zero is returned.
 *  @return     On error, -1 is returned.
 */
int check_valid_ipv4(const char *ipv4);

/**
 *	Check if endpoint is valid using regular expression below:
 *			"^(ipc|tcp)://.*$".
 *
 *	@param [in] endpoint    Endpoint string (e.g: "ipc:///tmp/feeds/0")
 *
 *  @return     Success, zero is returned.
 *  @return     On error, -1 is returned.
 */
int check_valid_endpoint(const char *endpoint);

#endif  /* UTILS_H_INCLUDED */
