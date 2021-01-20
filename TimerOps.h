#ifndef SIMPLEHTTP_TIMEROPS_H
#define SIMPLEHTTP_TIMEROPS_H

#include <sys/timerfd.h>

namespace Timer
{
	int createTimerfd(int clockid,int flags);
	
	void setTimer(int fd,int flags,const struct itimerspec *new_value,
                 struct itimerspec *old_value);	

	void getTimer(int fd,struct itimerspec *curr_value);	
}

#endif
