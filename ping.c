#include "ping.h" 

//1. send package

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

static void flagCases(int ac, char *av[])
{
	int ch;
	opterr = 0;
	while ((ch = getopt(ac, av, COMMON_OPTSTR)) != EOF) 
	{
		switch (ch)
		{
			case ':':
				exit (error(2, ERR1));
				break ;
			case '?':
				usage();
				break ;
			case 'v':
				printf ("la flag es v y tiene este arg %s\n", optarg);
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

	sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock_fd < 0)
	{
		if (geteuid() != 0)
			exit(error(1, "sudo permission required for raw sockets"));
		exit(error(1, "socket file descriptor not received"));
	}
	return sock_fd;
}

static void sendPing(void) 
{
	printf("--------Send ping-------\n");

	exit (error(1, av[pos]));		
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

static void ping(char *av[])
{
	int pos, sock_fd;
	char *ip_addr;
	struct sockaddr_in addr_con;

	pos = getAddr(av);
	ip_addr = dnsResolution(av[pos], &addr_con);
	if (!ip_addr)
		exit(error(2, ERR2));
	sock_fd = rawSocket();
	sendPing(sock_fd, &addr_con, ip_addr);
}

static void parser(int ac, char *av[])
{
	flagCases(ac, av);
}

int main (int ac, char *av[])
{
	if (ac != 1)
	{
		parser(ac, av);
		ping(av);
		return (0);
	} 
	return (error(2, ERR1));
}
