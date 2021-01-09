#include "Poller.h"
#include "PollPoller.h"
#include "EpollPoller.h"

#include <stdlib.h>

Poller::Poller(const int& listenFd)
  :  listenFd_(listenFd)
{
}

Poller::~Poller() = default;

Poller* Poller::getPoller(const int& listenFd)
{
	if(::getenv("SIMPLEHTTP_USE_POLL"))
		return new PollPoller(listenFd);
	else
		return new EpollPoller(listenFd);
}
