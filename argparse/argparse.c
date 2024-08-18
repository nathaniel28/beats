// under GPL-2.0

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argparse.h"

static int parse_option(Option *opt, char *set) {
	long res;
	switch (opt->type) {
	case OPT_CHAR:
		if (set[0] == '\0' || set[1] != '\0') {
			printf("\"%s\" is not a char\n", set);
			return -1;
		}
		*(char *) opt->dst = set[0];
		break;
	case OPT_LONG:
		errno = 0;
		res = strtol(set, NULL, 10);
		if (errno) {
			printf("Failed to parse \"%s\" as a long integer\n", set);
			return -1;
		}
		*(long *) opt->dst = res;
		break;
	case OPT_STR:
		*(char **) opt->dst = set;
		break;
	default:
		return -1;
	}
	return 0;
}

static Option *find_long_option(char *long_name, Option *opts, int len) {
	while (len--) {
		if (opts->long_name && strcmp(opts->long_name, long_name) == 0)
			return opts;
		opts++;
	}
	return NULL;
}

static Option *find_short_option(char short_name, Option *opts, int len) {
	while (len--) {
		if (opts->name == short_name)
			return opts;
		opts++;
	}
	return NULL;
}

int argparse(int argc, char **argv, Option *opts, int len) {
	char **not_options = argv;
	int not_options_len = 0;
	while (argc--) {
		char *s = *argv;
		if (*s == '-') {
			s++;
			if (*s == '-') {
				s++;
				Option *found = find_long_option(s, opts, len);
				if (!found) {
					printf("Unknown option --%s\n", s);
					return -1;
				}
				if (found->type == OPT_BOOL) {
					*(unsigned char *) found->dst = !*(unsigned char *) found->dst;
				} else {
					if (!argc--) {
						printf("--%s requires an argument\n", found->long_name);
						return -1;
					}
					argv++;
					if (parse_option(found, *argv)) {
						return -1;
					}
				}
			} else {
				while (*s) {
					Option *found = find_short_option(*s, opts, len);
					if (!found) {
						printf("Unknown option -%c\n", *s);
						return -1;
					}
					if (found->type == OPT_BOOL) {
						*(unsigned char *) found->dst = !*(unsigned char *) found->dst;
					} else {
						if (s[1] != '\0') {
							printf("Only the last short option can take an argument\n");
							return -1;
						}
						if (!argc--) {
							printf("-%c requires an argument\n", found->name);
							return -1;
						}
						argv++;
						if (parse_option(found, *argv)) {
							return -1;
						}
						break;
					}
					s++;
				}
			}
		} else {
			*not_options = s;
			not_options++;
			not_options_len++;
		}
		argv++;
	}
	return not_options_len;
}
