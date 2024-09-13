#ifndef UTIL_TIMER_H
#define UTIL_TIMER_H

#include <windows.h>

namespace Util {
	class Timer {
	public:
		void start()
		{
			mStartCount = GetTickCount();
		}

		int stop()
		{
			int endCount = GetTickCount();
			return endCount - mStartCount;
		}

	private:
		unsigned int mStartCount;
	};
}
#endif