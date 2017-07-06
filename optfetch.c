#include "optfetch.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h> /* malloc, free */

#ifdef DEBUG
void dbprintf(const char *s, ...) {
	va_list args;
	va_start(args, s);

	vfprintf(stderr, s, args);

	va_end(args);
}
#else
void dbprintf(const char *s, ...) {
	/* so the compiler doesn't complain */
	(void) s;
}
#endif


static int countopts(struct opttype *opts) {
	int i = 0;

	while (1) {
		/* longname and short name are both 0, OR */
		if (((opts[i].longname == NULL) && (opts[i].shortname == 0)) ||
		/* output type was unspecified OR */
		((opts[i].type == 0) || (opts[i].type > OPTTYPE_STRING)) ||
		/* nowhere to output data! */
		(opts[i].outdata == NULL)) {
			return i;
		}

		i++;
	}

	return 0;
}

static int get_option_index_short(char opt, char *potentialopts, int len) {
	int i;

	for (i = 0; i < len; i++) {
		if (potentialopts[i] == 0) {
			continue;
		}

		if (potentialopts[i] == opt) {
			return i;
		}
	}

	return -1;
}

static int get_option_index_long(char *opt, char **potentialopts, int len) {
	int i;

	for (i = 0; i < len; i++) {
		if (potentialopts[i] == NULL) {
			continue;
		}

		if (!strcmp(potentialopts[i], opt)) {
			return i;
		}
	}

	return -1;
}



signed char fetchopts(int *argc, char ***argv, struct opttype *opts) {
	int numopts;

	char **longopts;
	char *shortopts;
	char *curropt;

	/* max 5 digits (%l64u) on windows, * but only 4 (%llu) on unix (plus an EOF) */
#ifdef _WIN32
	char format_specifier[6];
#else
	char format_specifier[5];
#endif
	/* gotta save that extra byte of memory */

	struct opttype *wasinarg = NULL;

	/* char to take up less memory, unsigned so compiler doesn't complain */
	unsigned char oneoffset;

	int argindex, newargc = 0;

	/* new argument variable with the arguments that aren't options */
	char **newargv = malloc(sizeof(char*) * (*argc));

	int option_index;

	int i; /* Counter for loops */



	numopts = countopts(opts);

	if (!numopts) {
		return 0;
	}

	longopts = malloc(sizeof(char*) * numopts);
	shortopts = malloc(sizeof(char) * numopts);



	/* fill these up with opts.  That way they're easier to look up */
	for (i = 0; i < numopts; i++) {
		longopts[i] = opts[i].longname;
		shortopts[i] = opts[i].shortname;
	}

	/* start at 1 because 0 is the executable name */
	for (argindex = 1; argindex < *argc; argindex++) {
		if ((curropt = (*argv)[argindex]) == NULL) continue;

		/* Last argument was an option, now we're setting the actual value of that option */
		if (wasinarg != NULL) {
			switch (wasinarg->type) {
				/* We set the format specifier here then make
				 * one sscanf call with it.  We don't even need
				 * to cast it because it's already a pointer
				 * unless the user fucked something up which is
				 * their fault!
				 */
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

				/* Handled differently.  This is because %s expects a char*, and copies one buffer to
				 * the other.  This is an enormous waste because we would then have to allocate a buffer
				 * when we could just make the string point to the same place as the other string.
				 */
				case OPTTYPE_STRING:
					*(char**)(wasinarg->outdata) = curropt;
					wasinarg = NULL;
					format_specifier[0] = 0;
					continue;
			}
			sscanf(curropt, format_specifier, wasinarg->outdata);
			wasinarg = NULL;
			format_specifier[0] = 0;
		} else {
			/* Has the user manually demanded that the option-parsing end now? */
			if (!strcmp(curropt, "--")) {
				/* copy over the remaining arguments to newargv */
				for (i = argindex+1; i < *argc; i++) {
					newargv[newargc] = (*argv)[i];
					newargc++;
				}

				goto end;
			}

			/* in an option, getting warmer! */
			if (curropt[0] == '-') {
				/* was it a --foo or just a -foo? */
				if (curropt[1] == '-') {
					oneoffset = 2;
				} else {
					oneoffset = 1;
				}

				/* is it a short opt (e.g. -f) or a long one (e.g. -foo)? */
				if (strlen(curropt+oneoffset) == 1) {
					option_index = get_option_index_short(curropt[oneoffset], shortopts, numopts);
				/* nope */
				} else {
					option_index = get_option_index_long(curropt+oneoffset, longopts, numopts);
				}

				/* not an option */
				if (option_index == -1) {
					newargv[newargc++] = curropt;
					dbprintf("Faulty option %s.\n", curropt);
					continue;
				} else {
					/* it's a boolean option, so the next loop doesn't want to know about it */
					if ((opts[option_index]).type == OPTTYPE_BOOL) {
						*(int*)opts[option_index].outdata = 1;
						/* just to make sure */
						wasinarg = NULL;
					/* let the next loop get the value */
					} else {
						wasinarg = &opts[option_index];
					}
				}
			} else {
				newargv[newargc++] = curropt;
				dbprintf("Regular argument %s.\n", curropt);
			}
		}
	}

end:
	*argc = newargc;

	for (i = 1; i <= newargc; i++) {
		/* -1, because argv starts at 1 (with 0 as program name), but newargv starts at 0 */
		(*argv)[i] = newargv[i-1];
	}

	free(longopts);
	free(shortopts);
	free(newargv);

	return 1;
}
