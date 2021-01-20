#include "TimerOps.h"

#include <stdlib.h>

namespace Timer
{

/*
*clockid : CLOCK_REALTIME or CLOCKMONOTONIC
*flags : TFD_CLOEXEC and TFD_NONBLOCK can be selected
*/

int createTimerfd(int clockid,int flags)
{
	int timerfd = timerfd_create(clockid,flags);
	if(timerfd == -1)
	{
		::exit(1);
	}
	return timerfd;
}

/*
*if flags equals 0,means timing starts when setTimer was called
*/

void setTimer(int fd,int flags,const struct itimerspec *new_value,
              struct itimerspec *old_value)
{
	int ret = timerfd_settime(fd,flags,new_value,old_value);
	if(ret == -1)
	{
		::exit(1);
	}
}
	
void getTimer(int fd,struct itimerspec *curr_value)
{
	int ret = timerfd_gettime(fd,curr_value);
	if(ret == -1)
	{
		::exit(1);
	}
}

}//end namepsace
