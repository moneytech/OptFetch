#include "optfetch.h"
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char **argv) {
	int debug = false;
	char *name = NULL;
	float boat = 0.0;

	struct opttype opts[] = { {"debug", 'd', OPTTYPE_BOOL, &debug}, {"name", 'n', OPTTYPE_STRING, &name}, {"boat", 'b', OPTTYPE_FLOAT, &boat}, {0, 0, 0, 0} };

	fetchopts(&argc, &argv, opts);

	if (debug) {
		printf("Did debug.\n");
	}

	printf("Hi.  My name is %s.  What's yours?\n", name);

	printf("My boat is %f feet long.  How about yours?\n", boat);

	printf("Looks like I have %d argument%s left over.  Fancy that now!\n", argc, (argc == 1) ? "" : "s");

	if (argc) {
		printf("They are:\n");
		for (int i = 1; i <= argc; i++) {
			printf("* %s\n", argv[i]);
		}
	}

	return 0;
}
