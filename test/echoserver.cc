//a tcp echo server to test poller

#include "../SocketsOps.h"
#include "../Poller.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <unistd.h>
#include <vector>

int main(int argc,char* argv[])
{
	if(argc > 2)
	{
		perror("usage: ./echoserver [option] [port]\n");
		exit(1);
	}

	short port = 9527;
	if(argc == 2)
		port = static_cast<short>(atoi(argv[1]));

	std::vector<int> fds;
	int lfd = Sockets::createListenSocketOrDie(port);
	std::unique_ptr<Poller> poller(Poller::getPoller(lfd));

	while(true)
	{
		fds.clear();
		poller->poll(-1,&fds);
		for(int i = 0; i < fds.size(); i++)
		{
			if(fds[i] == lfd)
			{
				
				int cfd = Sockets::accept(lfd);
				if(!poller->addFd(cfd))
					::close(cfd);
			}
			else
			{
				char buf[1024];
				int n = ::read(fds[i],buf,sizeof buf);
				if(n == 0)
				{
					poller->removeFd(fds[i]);
					close(fds[i]);
				}
				else
				{
					::write(fds[i],buf,n);
				}				
			}
		}		
	}

	return 0;
}
