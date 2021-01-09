//base class inherited by PollPoller and EpollPoller

#ifndef SIMPLEHTTP_POLLER_H
#define SIMPLEHTTP_POLLER_H

#include "noncopyable.h"
#include <vector>

class Poller : noncopyable
{
public:
	typedef std::vector<int> pollfds;
	Poller(const int& listenFd);
	virtual ~Poller();

	virtual bool addFd(int fd) = 0;
	virtual bool removeFd(int fd) = 0;
	virtual void poll(int timeout,pollfds* fds) = 0;

	static Poller* getPoller(const int& listenFd);
protected:
	const int& listenFd_;
};

#endif //SIMPLEHTTP_POLLER_H
