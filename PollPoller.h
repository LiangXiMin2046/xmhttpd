#ifndef SIMPLEHTTP_POLLPOLLER_H
#define SIMPLEHTTP_POLLPOLLER_H

#include "Poller.h"
#include <poll.h>
#include <unordered_map>

class PollPoller  :  public Poller
{
public:

	PollPoller(const int& listenFd);
	~PollPoller() override;

	bool addFd(int fd) override;
	bool removeFd(int fd) override;
	void poll(int timeout,pollfds* fds) override;
	
private:
	typedef std::vector<struct pollfd> Fdlists;
	typedef std::unordered_map<int,int> FdIndexMap;

	struct pollfd curfd_;
	Fdlists	fdlists_;
	FdIndexMap idmap_;
};

#endif //SIMPLEHTTP_POLLPOLLER_H
