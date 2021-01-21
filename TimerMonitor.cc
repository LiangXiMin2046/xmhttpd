#include "TimerMonitor.h"

TimerMonitor::TimerMonitor()
  :  header_(new Node(-1,0)),
     tail_(new Node(-1,0)),
     count_(0)
{
	header_->next_ = tail_;
	tail_->pre_ = header_;
}

TimerMonitor::~TimerMonitor()
{
	Node* cur = header_->next_;
	while(cur != tail_)
	{
		Node* pre = cur->pre_;
		Node* ne = cur->next_;
		pre->next_ = ne;
		ne->pre_ = pre;
		delete cur;
		cur = ne;
		--count_;
	}
	delete header_;
	delete tail_;
}

/*
*Find out connections should be closed.
*The arguement limit means time limit.
*/

std::vector<int> TimerMonitor::checkTimeout(const time_t limit)
{
	std::vector<int> fds;
	Node* cur = header_->next_;
	time_t now = time(NULL);
	while(cur != tail_)
	{
		if(now - cur->lastArrive_ >= limit)
		{
			fds.push_back(cur->key_);
			cur = cur->next_;
		}
		else
		{
			break;
		}		
	}
	return fds;
}

/*
* remove node from linklist.
* the connection will be closed.
*/

void TimerMonitor::removeNode(int key)
{
	Node* cur = nodes_[key];
	nodes_.erase(key);

	Node* pre = cur->pre_;
	Node* ne = cur->next_;
	pre->next_ = ne;
	ne->pre_ = pre;
	
	--count_;
	delete cur;	
}

/*
*this function update the message arrived time of every connection.
*/

bool TimerMonitor::putIntoMonitor(int key,time_t lastArrive)
{
	if(!nodes_.count(key))
	{
		return insertTail(key,lastArrive);	
	}
	else
	{
		Node* keyNode = nodes_[key];
		keyNode->lastArrive_ = lastArrive;
		update(key);
		return true;
	}
}

/*
*return the time of last message. 
*/

time_t TimerMonitor::getArriveTime(int key)
{
	if(!nodes_.count(key))
		return 0;
	else
		return nodes_[key]->lastArrive_;
}


/*
*insert a new node into tail
*/

bool TimerMonitor::insertTail(int key,time_t lastArrive)
{
	Node* newTail = nullptr;
	try{
		 newTail = new Node(-1,0);
	}catch(const std::bad_alloc& e){
		return false;
	}

	++count_;
	tail_->key_ = key;
	tail_->lastArrive_ = lastArrive;
	tail_->next_ = newTail;
	newTail->pre_ = tail_;
	nodes_[key] = tail_;
	tail_ = newTail;
	return true;
}

/*
*update arrive time of a connection
*/

void TimerMonitor::update(int key)
{
	Node* cur = nodes_[key];

	//remove node from current position
	Node* preNode = cur->pre_;
	Node* neNode = cur->next_;
	preNode->next_ = neNode;
	neNode->pre_ = preNode;

	//insert pointer cur into tail
	Node* preTail = tail_->pre_;
	preTail->next_ = cur;
	cur->pre_ = preTail;
	cur->next_ = tail_;
	tail_->pre_ = cur;
}

/*
*return the number of the nodes
*/
int TimerMonitor::size() const
{
	return count_;
}
