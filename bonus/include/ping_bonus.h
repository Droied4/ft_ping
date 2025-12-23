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
# include <netinet/ip.h>

# define COMMON_OPTSTR ":?v:f:s:n:w:p:"
# define h_addr h_addr_list[0]
# define PORT_NO 0
# define RECV_TIMEOUT 1
# define ERR1 "usage error: Destination address required"
# define ERR2 "Name or service not known"
# define ERR3 "Setting socket options to TTL failed"
# define ERR4 "sendto: Network is unreachable"

extern volatile int loop;

typedef struct ping_pkt {
    struct icmphdr hdr;
	char *msg;
} t_pckt;

typedef struct config {
	int ping_sleep;
	int ping_pkt_size;
	bool resolve_dns;
	int max_send;
	char *payload;
} t_config;

typedef struct ping {
	int sock_fd;	
	char *ip_addr;
	char *ip_name;
	struct sockaddr_in addr_con;
	t_config conf;
	t_pckt pckt;
} t_ping;


#endif
