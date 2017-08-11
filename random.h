#ifndef __random_h__
#define __random_h__

class randomex
{
public:
	static void seed(int s);
	static int  rand();
	static int  rand(int nMin, int nMax);
private:
	static void shift();
	static unsigned long _seed;
};
#endif // __random_h__
