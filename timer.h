#ifndef _TIMER_H_
#define _TIMER_H_

#include <time.h>

namespace hstd {

class Timer {
private:
	clock_t nBefore_, nAfter_;
public:
	Timer() {}

	void begin() {
		nBefore_ = clock();
	}

	// ms
	float end() {
		nAfter_ = clock();

		return nAfter_ - nBefore_;
	}
};

} // namespace hstd

#endif // _TIMER_H_