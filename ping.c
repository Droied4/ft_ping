#include "ping.h" 

//Tasks
//1. improve error management
//2. Error ftping without arguments or  

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
				printf ("%s\n", ERR1);
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

static void parser(int ac, char *av[])
{
	flagCases(ac, av);
}

int main (int ac, char *av[])
{
	if (ac != 1)
	{
		parser(ac, av);
		return (0);
	} 
	printf ("%s\n", ERR1);
	return (2);
}
