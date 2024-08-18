// under GPL-2.0

#ifndef ARGPARSE_H
#define ARGPARSE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *dst;
	const char *long_name;
	int type;
	char name;
} Option;

#define OPT(_name, _long_name, _type, _dst) \
	{ _dst, _long_name, _type, _name }

#define OPT_BOOL 0
#define OPT_CHAR 1
#define OPT_LONG 2
#define OPT_STR 3

extern int argparse(int argc, char **argv, Option *opts, int len);

#ifdef __cplusplus
}
#endif

#endif
