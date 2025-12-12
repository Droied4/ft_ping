#include "ping_bonus.h" 

volatile int loop = 42;

//ARREGLAR EL RE LINK DEL BONUS make bonus  
//rellenar el payload con un valor hexadecimal no con un caracter y anadir comprobacion
//paso 4 disfrutar porque ya acabe jejeje

static void safeExit(t_ping *p)
{
	if (p->ip_addr)
		free(p->ip_addr);
	if (p->pckt.msg)
		free(p->pckt.msg);
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
		  "-v \t\t verbose output\n"
		  "-f \t\t flood ping\n"
		  "-s \t\t packet size\n"
		  "-n \t\t numeric output only\n"
		  "-w \t\t deadline\n"
		  "-p \t\t pattern\n");
	exit(1);
}

static void verboseMode(t_ping p)
{
	printf("ping: sock4.fd: %d (socktype: %s)\n", p.sock_fd, "SOCK_DGRAM");
	printf("ping: ai->ai_family: %s, ai->ai_canonname: '%s'\n", "AF_INET", p.ip_name);
}

static void	checkOpt(char *arg, t_ping *p)
{
	for (unsigned int i = 0; i < strlen(arg); i++)
	{
		if (!(arg[i] >= 48 && arg[i] <= 57))
		{
			error(1, "invalid argument");
			safeExit(p);
		}
	}
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
			case 'f':
				p->conf.ping_sleep = 0;
				break ;
			case 's':
				checkOpt(optarg, p);
				p->conf.ping_pkt_size = atoi(optarg); break ;	
			case 'n':
				p->conf.resolve_dns = true;
			break ;
			case 'w':
				checkOpt(optarg, p);
				p->conf.max_send = atoi(optarg);
			break ;
			case 'p': 
				p->conf.payload = *optarg;
			break ;
		}
	}
}

static char *dnsResolution(char *addr_host, struct sockaddr_in *addr_con)
{
	struct hostent *host_entity;
	char *ip = (char *)malloc(NI_MAXHOST * sizeof(char));
	if (!ip)
		return (NULL);
	if ((host_entity = gethostbyname(addr_host)) == NULL)
		return (NULL);

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

static int receiveLogic(int socket_fd, int msg_count, struct timespec time_start, char *ip_addr, int ttl_val, t_config conf)
{
	unsigned int raddr_len;
	char rbuf[sizeof(struct icmphdr) + conf.ping_pkt_size + sizeof(struct ip)];
	struct timespec time_end;
	double time_elapsed;
	struct sockaddr_in r_addr;

	raddr_len = sizeof(r_addr);
	if (recvfrom(socket_fd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)&r_addr, &raddr_len) > 0)
	{
		clock_gettime(CLOCK_MONOTONIC, &time_end);
		time_elapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec)) / 1000000.0;
		long double rtt_msec = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + time_elapsed;

		struct icmphdr *recv_hdr = (struct icmphdr *)rbuf;
		if (recv_hdr->type == 0 && recv_hdr->code == 0)
		{
			printf("%d bytes from %s : icmp_seq=%d ttl=%d time=%.1Lf ms\n", (int)(conf.ping_pkt_size + sizeof(struct icmphdr)), ip_addr, msg_count, ttl_val, rtt_msec);
			return (1);
		}
	}
	return (0);
}

static void pcktConfig(t_pckt *pckt, int *msg_count)
{
	pckt->hdr.type = ICMP_ECHO;
	pckt->hdr.code = 0;
	pckt->hdr.un.echo.id = getpid();
	pckt->hdr.un.echo.sequence = (*msg_count)++;
	pckt->hdr.checksum = checksum(pckt, sizeof(*pckt));
}

static void beforeLoop(t_ping *p, struct timeval *tv_out, int *ttl_val, t_pckt *pckt)
{
	//init tv_out
	tv_out->tv_sec = RECV_TIMEOUT;
	tv_out->tv_usec = 0;

	//configure Time to Live opt
	if (setsockopt(p->sock_fd, SOL_IP, IP_TTL, ttl_val, sizeof(*ttl_val)) != 0)
	{
		error(1, ERR3);
		safeExit(p);
	}

	//Configure timeout for receive
	setsockopt(p->sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv_out, sizeof(tv_out));

	//Configure pckt
    memset(pckt, 0, sizeof(*pckt));
    pckt->msg = malloc(p->conf.ping_pkt_size);
    if (!pckt->msg)
        safeExit(p);
    for (int i = 0; i < p->conf.ping_pkt_size - 1; i++)
        pckt->msg[i] = p->conf.payload;
}


static int sendPing(t_ping *p, t_pckt pckt, struct sockaddr_in *addr_con, t_config conf) 
{
	struct timeval tv_out;
	struct timespec time_start, tfs, tfe;
	long double total_msec = 0;
	double time_elapsed;
	int msg_count = 0, ttl_val = 64, msg_received_count = 0, socket_fd;
	char *ip_name;
	char *ip_addr;
	socket_fd = p->sock_fd;
	ip_name = p->ip_name;
	ip_addr = p->ip_addr;

	beforeLoop(p, &tv_out, &ttl_val, &pckt);
	clock_gettime(CLOCK_MONOTONIC, &tfs);
	if (conf.resolve_dns)
		printf("PING %s (%s) %d(%d) bytes of data\n", ip_addr, ip_addr, conf.ping_pkt_size, (int)(conf.ping_pkt_size + sizeof(struct icmphdr) + sizeof(struct ip)));
	else
		printf("PING %s (%s) %d(%d) bytes of data\n", ip_name, ip_addr, conf.ping_pkt_size, (int)(conf.ping_pkt_size + sizeof(struct icmphdr) + sizeof(struct ip)));
	while (loop)
	{
		//Config pckt 
		pcktConfig(&pckt, &msg_count);

		usleep(conf.ping_sleep);
		clock_gettime(CLOCK_MONOTONIC, &time_start);
		if (!loop || (conf.max_send > 0 && msg_received_count >= conf.max_send))
		{
			msg_count--;
			break ;
		}
		//send
		if (sendto(socket_fd, &pckt, sizeof(pckt), 0, (struct sockaddr *)addr_con, sizeof(*addr_con)) <= 0)
			return (error(2, ERR4));
		if (conf.ping_sleep > 0)
			msg_received_count += receiveLogic(socket_fd, msg_count, time_start, ip_addr, ttl_val, conf);
		else
			write(1, ".", 1);
	}
	clock_gettime(CLOCK_MONOTONIC, &tfe);
	time_elapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec)) / 1000000.0;
	total_msec = (tfe.tv_sec - tfs.tv_sec) * 1000.0 + time_elapsed;

	if (conf.resolve_dns)
		printf("\n--- %s ping statistics ---\n", ip_addr);
	else
		printf("\n--- %s ping statistics ---\n", ip_name);
	printf("%d packets transmitted, %d received, %.0f%% packet loss, time %.0Lfms\n", msg_count, msg_received_count, ((msg_count - msg_received_count) / (double)msg_count) * 100.0, total_msec);	
	return (0);
}

static int getAddr(char *av[])
{
	int i = 1;
	while(av[i + 1])
	{
		if (av[i][0] == '-')
		{
			if (!(av[i + 2]))
				return (i + 1);
			else
				return (i + 2);
		}
		i++;
	}
	return (i);
}

static void init(char *av[], t_ping *p)
{
	int pos;

	pos = getAddr(av);
	p->ip_name = strdup(av[pos]); p->conf.ping_sleep = 1000000;
	p->conf.ping_sleep = 1000000;
	p->conf.ping_pkt_size = 56;
	p->conf.resolve_dns = false;
	p->conf.max_send = 0;
	p->conf.payload = '0';
}


static void ping(int ac, char *av[])
{
	t_ping p;

	init(av, &p); 
	p.ip_addr = dnsResolution(p.ip_name, &p.addr_con);
	if (!p.ip_addr)
		exit(error(2, ERR2));
	p.sock_fd = rawSocket();
	if (p.sock_fd <= 0)
		safeExit(&p);
	signal(SIGINT, loop_handler);
	flagCases(ac, av, &p);
	sendPing(&p, p.pckt, &p.addr_con, p.conf);
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
