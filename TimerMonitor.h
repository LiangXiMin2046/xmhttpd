#ifndef SIMPLEHTTP_TIMERMONITOR_H
#define SIMPLEHTTP_TIMERMONITOR_H

/*
*by using LRU algorithm,TimerMonitor can help server to close those
*connections which are over time. 
*/

#include "noncopyable.h"

#include <time.h>
#include <unordered_map>
#include <vector>

class TimerMonitor : noncopyable
{
public:
	TimerMonitor();
	~TimerMonitor();
	std::vector<int> checkTimeout(const time_t limit);
	void removeNode(int key);
	bool putIntoMonitor(int key,time_t lastArrive);
	time_t getArriveTime(int key);
	int size() const;
private:
	struct Node
	{
		int key_;
		time_t lastArrive_;
		Node* pre_;
		Node* next_;
		Node(int key,time_t lastArrive)
			:  key_(key),
               lastArrive_(lastArrive),
               pre_(nullptr),
               next_(nullptr)
		{

		}
	};

private:
	bool insertTail(int key,time_t lastArrive);
	void update(int key);	
	
	Node* header_;
	Node* tail_;
	std::unordered_map<int,Node*> nodes_;
	int count_;
};

#endif //SIMPLEHTTP_TIMERMONITOR_H

