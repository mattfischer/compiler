#ifndef UTIL_UNIQUE_QUEUE_H
#define UTIL_UNIQUE_QUEUE_H

#include <queue>
#include <set>

namespace Util {
	template<typename T>
	class UniqueQueue {
	public:
		bool empty() { return mQueue.empty(); }

		T &front() { return mQueue.front(); }
		T &back() { return mQueue.back(); }

		void push(const T& t) {
			if(mSet.find(t) == mSet.end()) {
				mQueue.push(t);
				mSet.insert(t);
			}
		}

		void pop() {
			mSet.erase(mQueue.front());
			mQueue.pop();
		}

	private:
		std::queue<T> mQueue;
		std::set<T> mSet;
	};
}
#endif