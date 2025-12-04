#ifndef PING_BONUS_H
# define PING_BONUS_H

# include <stdio.h>
# include <time.h>
# include <netdb.h>
# include <signal.h>
# include <unistd.h>
# include <stdbool.h>
# include <stdlib.h>
# include <string.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>

# define COMMON_OPTSTR ":?v:f:"
# define h_addr h_addr_list[0]
# define PORT_NO 0
# define PING_PKT_S 64
# define RECV_TIMEOUT 1
# define ERR1 "usage error: Destination address required"
# define ERR2 "Name or service not known"
# define ERR3 "Setting socket options to TTL failed"
# define ERR4 "sendto: Network is unreachable"

extern volatile int loop;

struct ping_pkt {
    struct icmphdr hdr;
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
};

typedef struct config {
	int ping_sleep;
} t_config;

typedef struct ping {
	int sock_fd;	
	char *ip_addr;
	char *ip_name;
	struct sockaddr_in addr_con;
	t_config conf;
} t_ping;


#endif
