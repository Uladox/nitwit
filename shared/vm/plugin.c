#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

#define xstr(s) str(s)
#define str(s) #s
#define SYS_exit 60
#define STACK_SIZE 15625

struct plugin {
	char *name;
	int argc;
	char **argv;
	void *stack;
};

static int
load_plugin(void *arg)
{
	struct plugin *p = arg;
	int (*m)(int, char**);
	void *handle = dlopen(p->name, RTLD_LAZY);

	dlerror();

	/* Workaround that keeps using dlsym c99 complient */
	*(void **) &m = dlsym(handle, "main");
	m(p->argc, p->argv);
	printf("got here\n");
	dlclose(handle);

	/* int i = 0; */

	/* for(; i != p->argc; ++i) */
	/* 	free(p->argv[0]); */
	/* free(p->argv); */
	/* free(p->name); */
	/* free(p); */
	/* free(stack); */
	/* asm("movl $" xstr(SYS_exit) ", %eax\n" */
	/*     "syscall"); */
	/* Should not be reached since an exit call is made */
	return 0;
}

void
start_plugin(char *name)
{
	char *stack = malloc(STACK_SIZE);
	struct plugin *p = malloc(sizeof(*p));

	p->name = name;
	p->argc = 0;
	p->argv = NULL;
	p->stack = stack;

        clone(load_plugin, stack + STACK_SIZE,
	      CLONE_FILES | CLONE_VM |
	      CLONE_IO | CLONE_SYSVSEM, p);
}
