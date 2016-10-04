#include "Channel.h"

using namespace BaeServer;

int main(int argc, char **argv) {
	const size_t MIN_THREADS_PER_CHANNEL = 2;
	const size_t DEFAULT_PORT = 9000;
	
	size_t port 	= DEFAULT_PORT;
	size_t channels = MIN_THREADS_PER_CHANNEL;

	Channel::Run(port, channels);

	return 0;
}
