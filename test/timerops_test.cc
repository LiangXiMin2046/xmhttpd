//Timer test

#include "../TimerOps.h"
#include "../Poller.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <unistd.h>
#include <vector>
#include <time.h>
#include <stdint.h>
#include <string.h>

int main(int argc,char* argv[])
{
	std::vector<int> fds;
	int timerfd = Timer::createTimerfd(CLOCK_MONOTONIC,TFD_CLOEXEC | TFD_NONBLOCK);

	struct itimerspec nv;
	struct timespec start;
	::clock_gettime(CLOCK_MONOTONIC,&start);
	memset(&nv,0,sizeof nv);
	nv.it_value.tv_sec = 5;
	nv.it_interval.tv_sec = 5;
	Timer::setTimer(timerfd,0,&nv,NULL);
	std::unique_ptr<Poller> poller(Poller::getPoller(timerfd));

	while(true)
	{
		fds.clear();
		poller->poll(-1,&fds);
		for(int i = 0; i < fds.size(); i++)
		{
			if(fds[i] == timerfd)
			{
				uint64_t buf;
				::read(timerfd,&buf,sizeof buf);
				struct timespec now;
				::clock_gettime(CLOCK_MONOTONIC,&now);
				printf("%ds has passed\n",now.tv_sec - start.tv_sec);
			}	
		}		
	}

	return 0;
}
