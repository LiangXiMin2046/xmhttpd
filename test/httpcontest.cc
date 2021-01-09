//a tcp echo server to test poller

#include "../SocketsOps.h"
#include "../Poller.h"
#include "../HttpConnection.h"


#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <sys/socket.h>

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
	std::unordered_map<int,std::unique_ptr<HttpConnection>> m;
	while(true)
	{
		fds.clear();
		poller->poll(-1,&fds);
		printf("new message arrives\n");
		for(int i = 0; i < fds.size(); i++)
		{
			if(fds[i] == lfd)
			{
				
				int cfd = Sockets::accept(lfd);
				if(!poller->addFd(cfd))
					::close(cfd);
				else
					m.insert(std::make_pair(cfd,std::move(std::unique_ptr<HttpConnection>(new HttpConnection(cfd)))));
			}
			else
			{
				char buf[1024];
				int n = ::read(fds[i],buf,sizeof buf);
				if(n == 0)
				{
					std::cout << "client close" << std::endl;
					poller->removeFd(fds[i]);
					close(fds[i]);
				}
				else
				{
					m[fds[i]]->appendInputBuffer(buf,n);
					if(!m[fds[i]]->parseMessage())
					{
						//std::cout << "error happens" << std::endl;
						write(fds[i],"fail\n",6);
						//std::cout << "write happens" << std::endl;
						poller->removeFd(fds[i]);
						//std::cout << "remove happens" << std::endl;
						m.erase(fds[i]);
						//std::cout << "erase happens" << std::endl;
						//close(fds[i]);
						std::cout << m.size() << std::endl;
					}
					if(m.count(fds[i]) && m[fds[i]]->gotAll())
					{
						std::string request = m[fds[i]]->reqAsString();
						write(fds[i],request.data(),request.size());
						m[fds[i]]->reset();
					}
				}				
			}
		}		
	}

	return 0;
}
