#include "mydns.h"
// Test main function

int main() {
	struct packet* packet = make_query_packet("video.cs.cmu.edu");
	char data[MAX_BUFFER];
	int length;
	serialize(packet, data, &length);
	return 0;
}
