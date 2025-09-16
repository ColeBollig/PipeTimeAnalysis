#include <mach/mach_time.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <vector>

#define KB 1024
#define MB KB * KB
#define GB MB * KB

#define READ_PIPE 0
#define WRITE_PIPE 1

static uint64_t get_time_ns() {
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

void close_pipe(int fd) {
	if (close(fd) == -1) {
		fprintf(stderr, "Error (%d): Failed to close pipe %d\n", getpid(), fd);
		exit(1);
	}
}

int main(int argc, const char** argv) {
	uint64_t num_bytes = 4;

	if (argc > 2) {
		fprintf(stderr, "Error: Invalid number of arguments: %d\n", argc);
		exit(1);
	} else if (argc == 2) {
		std::string str(argv[1]);
		size_t pos = 0;
		uint64_t num = std::stoll(str, &pos);
		if (pos < str.length()) {
			if (str[pos] == 'k' || str[pos] == 'K') {
				num *= KB;
			} else if (str[pos] == 'm' || str[pos] == 'M') {
				num *= MB;
			} else if (str[pos] == 'g' || str[pos] == 'G') {
				num *= GB;
			}
		} 
		num_bytes = num;
	}

	// Note: Pipe Size: 65536 bytes = 64K
	int pipefd[2];
	if (pipe(pipefd) == -1) {
		fprintf(stderr, "Error: Failed to create pipe\n");
		exit(1);
	}

	std::vector<char> data(num_bytes, '-');
	std::vector<char> rdata(num_bytes);
	ssize_t bytes_read = 0;

	pid_t pid = fork();

	if (pid == -1) {
		fprintf(stderr, "Error: Fork failed\n");
		exit(1);
	} else if (pid) { // Parent
		close_pipe(pipefd[WRITE_PIPE]);

		uint64_t start = get_time_ns();

		while ((bytes_read = read(pipefd[READ_PIPE], rdata.data(), num_bytes)) > 0) {}

		uint64_t end = get_time_ns();

		if (bytes_read == -1) {
			fprintf(stderr, "Error (%d): Failed to read pipe\n", getpid());
			exit(1);
		}

		FILE* fp = fopen("read.out", "a");
		fprintf(fp ?: stderr, "%d: %llu | %llu\n", getpid(), num_bytes, end - start);

		close_pipe(pipefd[READ_PIPE]);
	} else { // Child
		close_pipe(pipefd[READ_PIPE]);

		uint64_t start = get_time_ns();

		if (write(pipefd[WRITE_PIPE], data.data(), num_bytes) == -1) {
			fprintf(stderr, "Error (%d): Failed to write data to pipe\n", getpid());
			exit(1);
		}

		uint64_t end = get_time_ns();

		FILE* fp = fopen("write.out", "a");
		fprintf(fp ?: stdout, "%d: %llu | %llu\n", getpid(), num_bytes, end - start);

		close_pipe(pipefd[WRITE_PIPE]);
	}

	return 0;
}

