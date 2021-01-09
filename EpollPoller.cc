#include "EpollPoller.h"

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

EpollPoller::EpollPoller(const int& listenFd)
  :  Poller(listenFd),
     epollfd_(epoll_create1(EPOLL_CLOEXEC))
{
	assert(epollfd_ != -1);
	assert(addFd(listenFd_));
	fdlists_.resize(ListsInitialSize);
}

EpollPoller::~EpollPoller()
{

}

bool EpollPoller::addFd(int fd)
{
	memset(&curfd_,0,sizeof curfd_);
	curfd_.events = EPOLLIN;
	curfd_.data.fd = fd;
	int ret = epoll_ctl(epollfd_,EPOLL_CTL_ADD,fd,&curfd_);
	return ret != -1;
}

bool EpollPoller::removeFd(int fd)
{
	memset(&curfd_,0,sizeof curfd_);
	curfd_.events = EPOLLIN;
	curfd_.data.fd = fd;
	int ret = epoll_ctl(epollfd_,EPOLL_CTL_DEL,fd,&curfd_);
	return ret != -1;
}

void EpollPoller::poll(int timeout,pollfds* fds)
{
	int num = epoll_wait(epollfd_,fdlists_.data(),static_cast<int>(fdlists_.size()),timeout);
	if(num < 0)
	{
		if(errno == EINTR)
		{
			return;
		}
		exit(1);
	}	

	//expand fdlists's size if needed
	if(static_cast<size_t>(num) == fdlists_.size())
		fdlists_.resize(2 * (fdlists_.size()));

	for(int i = 0; i < num; i++)
	{
		if(fdlists_[i].events & EPOLLIN)
		{
			fds->push_back(fdlists_[i].data.fd);
		}	
	}
}
