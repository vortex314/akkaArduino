
#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

class Semaphore {
	public:
		static Semaphore& create();
		virtual void wait() = 0;
		virtual void release() = 0;
};

#endif
