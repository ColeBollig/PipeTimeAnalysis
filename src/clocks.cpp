#include <stdio.h>
#include <chrono>
#include <time.h>
#include <sys/time.h>
#include <array>
#include <unistd.h>
#include <mach/mach_time.h>

#define NUM_TESTS 100
#define SLEEP_TIME 5

using system_t = std::chrono::time_point<std::chrono::system_clock>;
using steady_t = std::chrono::time_point<std::chrono::steady_clock>;

uint64_t get_time_ns() {
	static mach_timebase_info_data_t info;
	static bool init = false;

	if ( ! init) {
		mach_timebase_info(&info);
		init = true;
	}

	uint64_t time = mach_absolute_time();
	time = (time * info.numer) / info.denom;
	return time;
}

int main() {
	std::array<std::pair<timespec, timespec>, NUM_TESTS> gettimereal_data;
	std::array<std::pair<timespec, timespec>, NUM_TESTS> gettimemono_data;
	std::array<std::pair<timeval, timeval>, NUM_TESTS> timeofday_data;
	std::array<std::pair<system_t, system_t>, NUM_TESTS> sysclock_data;
	std::array<std::pair<steady_t, steady_t>, NUM_TESTS> steadyclock_data;
	std::array<std::pair<uint64_t, uint64_t>, NUM_TESTS> mach_data;

	for (auto& [start, end] : gettimereal_data) {
		clock_gettime(CLOCK_REALTIME, &start);
		sleep(SLEEP_TIME);
		clock_gettime(CLOCK_REALTIME, &end);
	}

	for (auto& [start, end] : gettimemono_data) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		sleep(SLEEP_TIME);
		clock_gettime(CLOCK_MONOTONIC, &end);
	}

	for (auto& [start, end] : timeofday_data) {
		gettimeofday(&start, nullptr);
		sleep(SLEEP_TIME);
		gettimeofday(&end, nullptr);
	}

	for (auto& [start, end] : sysclock_data) {
		start = std::chrono::system_clock::now();
		sleep(SLEEP_TIME);
		end = std::chrono::system_clock::now();
	}

	for (auto& [start, end] : steadyclock_data) {
		start = std::chrono::steady_clock::now();
		sleep(SLEEP_TIME);
		end = std::chrono::steady_clock::now();
	}

	for (auto& [start, end] : mach_data) {
		start = get_time_ns();
		sleep(SLEEP_TIME);
		end = get_time_ns();
	}

	printf("ITER          RGET          MGET           TOD          SYSC          STDC          MACH\n");

	std::array<uint64_t, 6> sum {0, 0, 0, 0, 0, 0};
	for (size_t i = 0; i < NUM_TESTS; i++) {
		printf("%4zu", i);

		const auto& rgett = gettimereal_data[i];
		uint64_t elapsed_ns = (uint64_t)((rgett.second.tv_sec - rgett.first.tv_sec) * 1'000'000'000L + (rgett.second.tv_nsec - rgett.first.tv_nsec));
		printf("  %12llu", elapsed_ns);
		sum[0] += elapsed_ns;

		const auto& mgett = gettimemono_data[i];
		elapsed_ns = (uint64_t)((mgett.second.tv_sec - mgett.first.tv_sec) * 1'000'000'000L + (mgett.second.tv_nsec - mgett.first.tv_nsec));
		printf("  %12llu", elapsed_ns);
		sum[1] += elapsed_ns;

		const auto& tod = timeofday_data[i];
		long seconds = tod.second.tv_sec - tod.first.tv_sec;
		long microsec = tod.second.tv_usec - tod.first.tv_usec;
		if (microsec < 0) {
			seconds--;
			microsec += 1000000;
		}
		elapsed_ns = (uint64_t)((seconds * 1'000'000'000L) + (microsec * 1'000L));
		printf("  %12llu", elapsed_ns);
		sum[2] += elapsed_ns;

		const auto& sysc = sysclock_data[i];
		elapsed_ns = (uint64_t)(std::chrono::duration_cast<std::chrono::nanoseconds>(sysc.second - sysc.first).count());
		printf("  %12llu", elapsed_ns);
		sum[3] += elapsed_ns;

		const auto& steadyc = steadyclock_data[i];
		elapsed_ns = (uint64_t)(std::chrono::duration_cast<std::chrono::nanoseconds>(steadyc.second - steadyc.first).count());
		printf("  %12llu", elapsed_ns);
		sum[4] += elapsed_ns;

		const auto& mach = mach_data[i];
		elapsed_ns = mach.second - mach.first;
		printf("  %12llu", elapsed_ns);
		sum[5] += elapsed_ns;

		printf("\n");
	}

	printf("\nAVG:");
	for (const auto& n : sum) {
		printf("  %12llu", (n / NUM_TESTS));
	}

	printf("\n");

	return 0;
}

