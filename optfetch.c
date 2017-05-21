#include "optfetch.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>


uint32_t countopts(struct opttype *opts) {
	uint32_t i = 0;

	while (true) {
		// longname and short name are both 0, OR
		if (((opts[i].longname == NULL) && (opts[i].shortname == 0)) ||
		// output type was unspecified OR
		((opts[i].type == 0) || (opts[i].type > 15)) ||
		// nowhere to output data!
		(opts[i].outdata == NULL)) {
			return i;
		}

		i++;
	}

	return 0;
}

int32_t get_option_index_short(char opt, char *potentialopts, uint32_t len) {
	for (uint32_t i = 0; i < len; i++) {
		if (potentialopts[i] == opt) {
			return i;
		}
	}

	return -1;
}

int32_t get_option_index_long(char *opt, char **potentialopts, uint32_t len) {
	for (uint32_t i = 0; i < len; i++) {
		if (strcmp(potentialopts[i], opt)) {
			return i;
		}
	}

	return -1;
}



uint32_t fetchopts(int argc, char **argv, struct opttype *opts) {
	char **longopts;
	char *shortopts;
	char *curropt;

	// %Lf long double
	// %lf double
	// %f float
	// %u unsigned int
	// %d int
	// max 5 digits (%l64u), plus an EOF
	char format_specifier[6];

	struct opttype *wasinarg = NULL;

	char oneoffset;
	uint32_t newargc;
	int32_t option_index;
	uint32_t numopts = countopts(opts);
	if (!numopts) {
		return -1;
	}
	longopts = malloc(sizeof(char*) * numopts);
	shortopts = malloc(sizeof(char) * numopts);

	// fill these up with opts.  That way they're easier to look up
	for (uint32_t i = 0; i < numopts; i++) {
		longopts[i] = opts[i].longname;
		shortopts[i] = opts[i].shortname;
	}

	// start at 1 because 0 is the executable name
	for (newargc = 1; newargc < argc; newargc++) {
		curropt = argv[newargc];

		// Last argument was an option, now we're setting the actual value of that option
		if (wasinarg != NULL) {
			switch (wasinarg->type) {
				// We set the format specifier here then make
				// one sscanf call with it.  We don't even need
				// to cast it because it's already a pointer
				// unless the user fucked something up which is
				// their fault!
				case OPTTYPE_CHAR: strcpy(format_specifier, "%c"); break;
				case OPTTYPE_SHORT: strcpy(format_specifier, "%hi"); break;
				case OPTTYPE_USHORT: strcpy(format_specifier, "%hu"); break;
				case OPTTYPE_INT: strcpy(format_specifier, "%d"); break;
				case OPTTYPE_UINT: strcpy(format_specifier, "%u"); break;
				case OPTTYPE_LONG: strcpy(format_specifier, "%ld"); break;
				case OPTTYPE_ULONG: strcpy(format_specifier, "%lu"); break;
#ifdef _WIN32
				case OPTTYPE_LONGLONG: strcpy(format_specifier, "%l64d"); break;
				case OPTTYPE_ULONGLONG: strcpy(format_specifier, "%l64u"); break;
#else
				case OPTTYPE_LONGLONG: strcpy(format_specifier, "%lld"); break;
				case OPTTYPE_ULONGLONG: strcpy(format_specifier, "%llu"); break;
#endif
				case OPTTYPE_FLOAT: strcpy(format_specifier, "%f"); break;
				case OPTTYPE_DOUBLE: strcpy(format_specifier, "%lf"); break;
				case OPTTYPE_LONGDOUBLE: strcpy(format_specifier, "%Lf"); break;
	
				// Handled differently
				case OPTTYPE_STRING:
					wasinarg->outdata = curropt;
					continue;
			}
			sscanf(curropt, format_specifier, wasinarg->outdata);
			format_specifier[0] = 0;
			wasinarg = NULL;
		} else {
			// Has the user manually demanded that the option-parsing end now?
			if (strcmp(curropt, "--")) {
				free(longopts);
				free(shortopts);
				newargc++;
				return newargc;
			}

			// in an option, getting warmer!
			if (curropt[0] == '-') {
				// was it a --foo or just a -foo?
				if (curropt[1] == '-') {
					oneoffset = 2;
				} else {
					oneoffset = 1;
				}

				// is it a short opt (e.g. -f) or a long one (e.g. -foo)?
				if (strlen(curropt+oneoffset) == 1) {
					option_index = get_option_index_short(curropt[oneoffset], shortopts, numopts);
				// nope
				} else {
					option_index = get_option_index_long(curropt+oneoffset, longopts, numopts);
				}

				// not an option
				if (option_index == -1) {
					continue;
				} else {
					// it's a boolean option, so the next loop doesn't want to know about it
					if ((opts[option_index]).type == OPTTYPE_BOOL) {
						*(bool*)opts[option_index].outdata = 1;
						// just to make sure
						wasinarg = NULL;
					// let the next loop get the value
					} else {
						wasinarg = &opts[option_index];
					}
				}
			}
		}
	}

	free(longopts);
	free(shortopts);

	return newargc;
}
