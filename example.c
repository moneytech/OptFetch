#include "optfetch.h"
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char **argv) {
	bool debug = false;
	char *name = NULL;
	float boat;

	struct opttype opts[] = { {"debug", 'd', OPTTYPE_BOOL, &debug}, {"name", 'n', OPTTYPE_STRING, &name}, {"boat", 'b', OPTTYPE_FLOAT, &boat}, {0, 0, 0, 0} };

	int c = fetchopts(argc, argv, opts);

	if (debug) {
		printf("Did debug.\n");
	}

	printf("Hi.  My name is %s.  What's yours?\n", name);

	printf("My boat is %f feet long.  How about yours?\n", boat);

	printf("Looks like I have %d arguments left over.  Fancy that now!\n", argc - c);

	return 0;
}
