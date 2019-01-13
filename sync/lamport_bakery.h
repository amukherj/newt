#ifndef SYNC_LAMPORT_BAKERY_H
#define SYNC_LAMPORT_BAKERY_H

#include <unordered_map>
#include <thread>
#include <cassert>
#include <atomic>
#include <bitset>
#include <algorithm>
#include <vector>
#include <functional>
#include <chrono>
#include <sstream>
#include <iostream>

namespace sync {

template <int N>
class LamportBakery {
public:
	LamportBakery() : queued{0}, tokens{0}, max_id{0} {}

	void lock() {
		int thr_id = thread_id();

		// enter the doorway and acquire a token
		queued[thr_id] = true;
		tokens[thr_id] = *std::max_element(tokens, tokens + N) + 1;
		queued[thr_id] = false;

		// await your turn by allowing all threads in the doorway to acquire
		// a token, and then waiting for all threads with lower token numbers
		// (or same token and lower id) to be done first.
		for (int i = 0; i < N; ++i) {
			if (i != thr_id) {
				while (queued[i]) {
					std::this_thread::yield();
				}

				while (tokens[i] > 0 && std::make_pair(tokens[i], i) <
					std::make_pair(tokens[thr_id], thr_id)) {
						std::this_thread::yield();
					}
			}
		}
	}

	void unlock() {
		// release the token
		int thr_id = thread_id();
		tokens[thr_id] = 0;
	}

	void dump() {
		std::stringstream sout;
		for (auto& e: thread_id_map) {
			sout << e.first << ':' << e.second << '\n';
		}
		std::cout << sout.str();
	}

private:
	// queue for threads
	std::bitset<N> queued;

	// per thread token
	int tokens[N];

	// max thread numeric id so far
	std::atomic<int> max_id;

	// map of thread id to numeric id
	std::unordered_map<std::thread::id, int> thread_id_map;

	// Get the numeric id of the thread
	int thread_id() {
		int n;
		auto thr_key = std::this_thread::get_id();

		if (thread_id_map.find(thr_key) == thread_id_map.end()) {
			n = ++max_id;
			thread_id_map[thr_key] = n;
		} else {
			n = thread_id_map[thr_key];
		}

		// assert(thread_id_map.size() == max_id);
		return n;
	}
};

} // namespace sync

#endif /* SYNC_LAMPORT_BAKERY_H */
