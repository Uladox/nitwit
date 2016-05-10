#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <threadpass.h>

#include "plugin.h"

#define xstr(s) str(s)
#define str(s) #s
/* #define SYS_exit 60 */
#define STACK_SIZE 15625

static void *
load_ntwt_plugin(void *arg)
{
	int (*m)(int, char**);
	struct ntwt_plugin *plugin = arg;
	void *handle = dlopen(plugin->name, RTLD_LAZY);

	dlerror();

	/* Workaround that keeps using dlsym c99 complient */
	*(void **) &m = dlsym(handle, "main");

	m(plugin->argc, plugin->argv);
	if (dlclose(handle)) {
		fprintf(stderr,
			"Error: Failed to close library %s\n",
			plugin->name);
		exit(1);
	}

        *(struct ntwt_plugin **) thread_pass_get(plugin->pass) = plugin;
	thread_pass_return(plugin->pass);

	pthread_exit(NULL);
}

void
ntwt_plugin_start(struct thread_pass *pass, char *name)
{
	pthread_attr_t attr;
	struct ntwt_plugin *plugin = malloc(sizeof(*plugin));

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, STACK_SIZE);

	plugin->name = name;
	plugin->pass = pass;
	plugin->argc = 0;
	plugin->argv = NULL;
	pthread_create(&plugin->thread, NULL, load_ntwt_plugin, plugin);
	pthread_attr_destroy(&attr);
}
