//Timer test

#include "../TimerOps.h"
#include "../Poller.h"
#include "../TimerMonitor.h"

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <memory>
#include <unistd.h>
#include <vector>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

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
	std::unique_ptr<TimerMonitor> monitor(std::move(new TimerMonitor));

	//test TimerMonitor::insertTail
	time_t now = time(NULL);
	for(int i = 0; i < 100; i++)
	{
		monitor->putIntoMonitor(i,now + i);
	}

	//test TimerMonitor::update
	for(int i = 0; i < 100; i++)
	{
		monitor->putIntoMonitor(i,now + i);
	}

	std::cout << "there are " << monitor->size() << " nodes" << std::endl;
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
				int cnt = monitor -> checkTimeout(5);		
				std::vector<int> result = monitor->removeNodes(cnt);
				//std::cout << result.size() << std::endl;
				for(int i = 0; i < 	result.size(); i++)
				{
					std::cout << "key = " << result[i] << "has deleted" << std::endl;
				}
				std::cout << "there are " << monitor->size() << " nodes" << std::endl;	
			}		
		}	
		if(monitor->size() == 0)
		{
			std::cout << "no nodes to be deleted" << std::endl;
		}	
	}

	return 0;
}
