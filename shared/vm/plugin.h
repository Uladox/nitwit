/* Include these
 * #include <unistd.h>
 * #include <threadpass.h>
 */

struct ntwt_plugin {
	struct thread_pass *pass;
        pthread_t thread;
	char *name;
	int argc;
	char **argv;
};

void ntwt_plugin_start(struct thread_pass *pass, char *name);

