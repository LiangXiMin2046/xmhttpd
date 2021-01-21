//create daemon process and http server start

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "HttpServer.h"

int main(void)
{
	switch(fork())
	{
		case -1:
			::exit(-1);
		case 0:
			break;
		default:
			::exit(0);
	}

	if(setsid() == -1)
		::exit(-1);

	umask(0);	

	switch(fork())
	{
		case -1:
			::exit(-1);
		case 0:
			break;
		default:
			::exit(0); 
	}						

	HttpServer server(80); //run on port 80 for http
	server.start();	
	return 0;
}
