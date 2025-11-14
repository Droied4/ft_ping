#ifndef PING_H
# define PING_H

# include <stdio.h>
# include <netdb.h>
# include <unistd.h>
# include <stdbool.h>
# include <stdlib.h>
# include <string.h>
# include <arpa/inet.h>

# define COMMON_OPTSTR ":?v:"
# define h_addr h_addr_list[0]
# define PORT_NO 0
# define ERR1 "usage error: Destination address required"
# define ERR2 "Name or service not know"

#endif
