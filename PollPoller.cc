#include "PollPoller.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <string.h>

PollPoller::PollPoller(const int& listenfd)
  :  Poller(listenfd)
{
	assert(addFd(listenFd_));
}

PollPoller::~PollPoller()
{
}

//add the fd of client in poller
bool PollPoller::addFd(int fd)
{
	if(idmap_.count(fd))
	{
		perror("trying to add a exisited fd!\n");
		return false;		
	}
	
	memset(&curfd_,0,sizeof curfd_);
	curfd_.fd = fd;
	curfd_.events = POLLIN;		

	fdlists_.push_back(curfd_);

	idmap_[fd] = static_cast<int>(fdlists_.size()) - 1;

	return true;//add successfully
}

//remove the fd of client in poller
bool PollPoller::removeFd(int fd)
{
	if(!idmap_.count(fd))
	{
		perror("trying to remove a non-existent fd!\n");
		return false;
	}

	int id = idmap_[fd];
	std::swap(fdlists_[id],fdlists_.back());
	fdlists_.pop_back();
	
	idmap_.erase(fd);

	return false;
}

//poll function,get fd when event happens
//blocked when timeout equals -1
 
void PollPoller::poll(int timeout,pollfds* fds)
{
	int n = ::poll(fdlists_.data(),fdlists_.size(),timeout);
	if(n < 0)
	{
		if(errno == EINTR)
		{
			return;
		}
		perror("error in poll");
		exit(1);
	}

	if(n == 0)
		return;		
	for(const auto& r : fdlists_)
	{
		if(r.revents & POLLIN)
		{
			fds->push_back(r.fd);
			if(--n == 0)
				break;
		}
	}
}
