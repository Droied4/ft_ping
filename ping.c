#include "ping.h" 

volatile int loop = 42;

static void safeExit(t_ping *p)
{
	if (p->ip_addr)
		free(p->ip_addr);
	if (p->ip_name)
		free(p->ip_name);
	if (p->sock_fd > 0)
	{
		close(p->sock_fd);	
		p->sock_fd = -1;
	}
	exit(1);
}

static int error(int error_code, char *msg)

{
	dprintf(2, "ping: %s\n", msg);
	return (error_code);
}

static void usage(void)
{
	printf("Usage\n" 
		  "\tping [options] <destination>\n"
		  "Options:\n" 
		  "-v \t\t verbose output\n");
	exit(1);
}

static void verboseMode(t_ping p)
{
	printf("ping: sock4.fd: %d (socktype: %s)\n", p.sock_fd, "SOCK_DGRAM");
	printf("ping: ai->ai_family: %s, ai->ai_canonname: '%s'\n", "AF_INET", p.ip_name);
}

static void flagCases(int ac, char *av[], t_ping *p)
{
	int ch;
	opterr = 0;
	while ((ch = getopt(ac, av, COMMON_OPTSTR)) != EOF) 
	{
		switch (ch)
		{
			case ':':
				error(2, ERR1);
				safeExit(p);
				break ;
			case '?':
				usage();
				break ;
			case 'v':
				verboseMode(*p);
				break ;	
		}
	}
}

static char *dnsResolution(char *addr_host, struct sockaddr_in *addr_con)
{
	struct hostent *host_entity;
	char *ip;
	ip = (char *)malloc(NI_MAXHOST * sizeof(char));
	if (!ip)
		return (NULL);
	if ((host_entity = gethostbyname(addr_host)) == NULL)
	{
		free(ip);
		return (NULL);
	}

	strcpy(ip, inet_ntoa(*(struct in_addr *)host_entity->h_addr));
	(*addr_con).sin_family = host_entity->h_addrtype;
	(*addr_con).sin_port = htons(PORT_NO);
	(*addr_con).sin_addr.s_addr = *(long *)host_entity->h_addr;  
	return (ip);
}	

static int rawSocket(void) 
{
	int sock_fd;

	sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
	if (sock_fd < 0)
		return (error(-1, "socket file descriptor not received"));
	return sock_fd;
}

//calculate the checksum (RFC 1071)
static unsigned short checksum (void *b, int len)
{
	unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

//signal handler
static void loop_handler(int sig)
{
	loop = 0;
	(void)sig;
}

static int sendPing(int socket_fd, struct sockaddr_in *addr_con, char *ip_addr, char *ip_name) 
{
	struct ping_pkt pckt;
	struct sockaddr_in r_addr;
	struct timeval tv_out;
	struct timespec time_start, time_end, tfs, tfe;
	char rbuf[128];
	unsigned int raddr_len;
	long double total_msec = 0;
	double time_elapsed;
	int msg_count = 0, i, ttl_val = 64, msg_received_count = 0;
	//init tv_out
	tv_out.tv_sec = RECV_TIMEOUT;
	tv_out.tv_usec = 0;
	
	clock_gettime(CLOCK_MONOTONIC, &tfs);
	//configure Time to Live opt
	if (setsockopt(socket_fd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
		return (error(1, ERR3));
	//Configure timeout for receive
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv_out, sizeof(tv_out));

	printf("PING %s (%s) %d(%d) bytes of data\n", ip_name, ip_addr, PING_PKT_S, PING_PKT_S + 8 + 20);
	while (loop)
	{
		//set pckt 
		bzero(&pckt, sizeof(pckt));
		pckt.hdr.type = ICMP_ECHO;
		pckt.hdr.un.echo.id = getpid();

		for (i = 0; i < (int)sizeof(pckt.msg) - 1; ++i)
			pckt.msg[i] = i + '0';

		pckt.msg[i] = 0;
		pckt.hdr.un.echo.sequence = msg_count++;
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

		usleep(PING_SLEEP);
		clock_gettime(CLOCK_MONOTONIC, &time_start);
		if (!loop)
		{
			msg_count--;
			break ;
		}
		//send
		if (sendto(socket_fd, &pckt, sizeof(pckt), 0, (struct sockaddr *)addr_con, sizeof(*addr_con)) <= 0)
			return (error(2, ERR4));
		//receive
		raddr_len = sizeof(r_addr);
		if (recvfrom(socket_fd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)&r_addr, &raddr_len) > 0)
		{
			clock_gettime(CLOCK_MONOTONIC, &time_end);
			time_elapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec)) / 1000000.0;
			long double rtt_msec = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + time_elapsed;
			struct icmphdr *recv_hdr = (struct icmphdr *)rbuf;
			if (recv_hdr->type == 0 && recv_hdr->code == 0)
			{
				printf("%d bytes from %s : icmp_seq=%d ttl=%d time=%.1Lf ms\n", PING_PKT_S, ip_addr, msg_count, ttl_val, rtt_msec);
				msg_received_count++;
			}
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &tfe);
	time_elapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec)) / 1000000.0;
	total_msec = (tfe.tv_sec - tfs.tv_sec) * 1000.0 + time_elapsed;

	printf("\n--- %s ping statistics ---\n", ip_name);

	printf("%d packets transmitted, %d received, %.0f%% packet loss, time %.0Lfms\n", msg_count, msg_received_count, ((msg_count - msg_received_count) / (double)msg_count) * 100.0, total_msec);	
	return (0);
}

static int getAddr(char *av[])
{
	int i = 1;
	while(av[i])
	{
		if (av[i][0] != '-')
			return (i);
		i++;
	}
	exit(error(2, ERR1));	
	return (-1);
}

static void ping(int ac, char *av[])
{
	t_ping p;
	int pos;

	p.ip_name = NULL;
	p.ip_addr = NULL;
	pos = getAddr(av);
	p.ip_name = strdup(av[pos]);
	p.ip_addr = dnsResolution(p.ip_name, &p.addr_con);
	if (!p.ip_addr)
	{
		error(2, ERR2);
		safeExit(&p);
	}
	p.sock_fd = rawSocket();
	if (p.sock_fd <= 0)
		safeExit(&p);
	signal(SIGINT, loop_handler);
	flagCases(ac, av, &p);
	sendPing(p.sock_fd, &p.addr_con, p.ip_addr, p.ip_name);
	safeExit(&p);
}

int main (int ac, char *av[])
{
	if (ac != 1)
	{
		ping(ac, av);
		return (0);
	} 
	return (error(2, ERR1));
}
