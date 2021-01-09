#ifndef SIMPLEHTTP_EPOLLPOLLER_H
#define SIMPLEHTTP_EPOLLPOLLER_H

#include "Poller.h"

#include <sys/epoll.h>

class EpollPoller : public Poller
{
public:

	EpollPoller(const int& listenfd);
	~EpollPoller() override;
	
	bool addFd(int fd) override;
	bool removeFd(int fd) override;
	void poll(int timeout,pollfds* fds) override;

private:
	typedef std::vector<struct epoll_event> FdLists;

	struct epoll_event curfd_;
	FdLists fdlists_;

	int epollfd_;

	const int ListsInitialSize = 64;
};

 
#endif //SIMPLEHTTP_EPOLLPOLLER_H
