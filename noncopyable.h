//a useful method to make your object uncopyable in c++11
//this method is metioned in <<Effective c++>>

#ifndef SIMPLEHTTP_NONCOPYABLE_H
#define SIMPLEHTTP_NONCOPYABLE_H

class noncopyable
{
public:
	noncopyable(const noncopyable& rhs) = delete;
	noncopyable& operator= (const noncopyable& rhs) = delete;
protected:
	noncopyable() = default;
	~noncopyable() = default;
};

#endif //ifndef SIMPLEHTTP_NONCOPYABLE_H
