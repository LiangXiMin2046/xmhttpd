#include "SocketsOps.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

namespace Sockets
{

int createSocketOrDie()
{
	int fd = ::socket(AF_INET,SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,IPPROTO_TCP);
	if(fd < 0)
	{
		perror("error on socket\n");
		exit(1);
	}
	return fd;
}

void bindOrDie(int fd,struct sockaddr* addr)
{
	int ret = ::bind(fd,addr,static_cast<socklen_t>(sizeof (struct sockaddr_in)));
	if(ret < 0)
	{
		perror("error on bind\n");
		exit(1);
	}
}

void listenOrDie(int fd)
{
	int ret = ::listen(fd,SOMAXCONN);
	if(ret < 0)
	{
		perror("error on listen");
		exit(1);
	}	
}

int accept(int fd)
{
	struct sockaddr_in addr;
	memset(&addr,0,sizeof addr);

	socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
	int connfd = ::accept4(fd,(struct sockaddr*)&addr,&addrlen,SOCK_CLOEXEC | SOCK_NONBLOCK);
	if(connfd < 0)
	{
		int curErrno = errno;
		perror("error on accept");
	}
	return connfd;	
}

int createListenSocketOrDie(uint16_t port)
{
	int lfd = createSocketOrDie();

	int on = 1;
	if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	{
		perror("error on setsockopt");
		exit(1);
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	bindOrDie(lfd,(struct sockaddr*)&addr);
	listenOrDie(lfd);
	return lfd;	
}

} //end namespace::Sockets
