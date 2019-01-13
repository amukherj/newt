#include "lamport_bakery.h"

int main() {
	constexpr int N = 4;
	sync::LamportBakery<N> pm;
	std::function<void()> thrfunc = [&pm] {
		pm.lock();
		std::cout << "Acquired lock in thread\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		std::cout << "Releasing lock in thread\n";
		pm.unlock();
	};
	std::vector<std::thread> threads;
	for (int i = 0; i < N-1; ++i) {
		threads.emplace_back(thrfunc);
	}

	// main thread's critical section
	pm.lock();
	std::cout << "Acquired lock\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	std::cout << "Releasing lock\n";
	pm.unlock();

	for (auto& t: threads) {
		t.join();
	}
}
