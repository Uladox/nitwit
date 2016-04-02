#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>

#include "practise.h"
#include "../nitwit_macros.h"
#include "../vm/vm.h"

static void test_fnct(double *can_happen,
		      double *strength,
		      double *unsatisfied)
{
	if (*can_happen * *strength > (1.0 - *unsatisfied)) {
		printf("This is a function!\n");
		*unsatisfied -= (1.0 - *strength);
	} else {
		*unsatisfied += *strength;
	}
}

static struct ntwt_action ntwt_built_in_actions[] = {
	[0] = { .loaded = 1,
		.package_num = 0,
		.id = 0,
		.name = (uint8_t *) u8"test",
		.funct = test_fnct }
};

struct ntwt_package ntwt_std_package = {
	.loaded = 1,
	.package_num = 0,
	.location = (uint8_t *) u8"",
	.handle = NULL,
	.action_ptr = ARRAY_SIZE(ntwt_built_in_actions),
	.action_max = ARRAY_SIZE(ntwt_built_in_actions),
	.actions = ntwt_built_in_actions
};

struct ntwt_practise *ntwt_practise_new(struct ntwt_action *action,
					double can_happen,
					double strength,
					double unsatisfied)
{
	struct ntwt_practise *prac = malloc(sizeof(*prac));

	prac->action = action;
	prac->can_happen = can_happen;
	prac->strength = strength;
	prac->unsatisfied = unsatisfied;
	pthread_mutex_init(&prac->done_mutex, NULL);

	return prac;
}

struct ntwt_action *ntwt_action_new(uint8_t *name,
				    uint32_t package_num,
				    uint32_t id,
				    void (*funct)(double *,
						  double *,
						  double *))
{
	struct ntwt_action *action = malloc(sizeof(*action));

	action->name = name;
	action->package_num = package_num;
	action->id = id;
	action->funct = funct;

	return action;
}

void ntwt_package_load_action(struct ntwt_package *package,
			      uint32_t id,
			      uint8_t *action_name)
{
	struct ntwt_action *action;
	char *error;

	if (id >= package->action_max) {
		fprintf(stderr,
			"Error loading action \"%s\" from package \"%s\": above max number of actions in package.",
			action_name, package->location);
		exit(1);
	}
	action = package->actions + id;
	action->funct = dlsym(package, (char *) action_name);
	error = dlerror();
	if (error) {
		fprintf(stderr,
			"Error loading action \"%s\" from package \"%s\": ",
			action_name, package->location);
		exit(1);
	}
	action->package_num = package->package_num;
	action->id = id;
	action->name = action_name;
	/* action->name = malloc(strlen(action_name) + 1); */
	/* strcpy(action->name, action_name); */
	if (id >= package->action_ptr)
		package->action_ptr = id + 1;
	action->loaded = 1;
};

void ntwt_practise_load(struct ntwt_practise *p,
			struct ntwt_action *action,
			double can_happen,
			double strength,
			double unsatisfied)
{
	p->loaded = 1;
	p->action = action;
	p->can_happen = can_happen;
	p->strength = strength;
	p->unsatisfied = unsatisfied;
	pthread_mutex_init(&p->done_mutex, NULL);
}

void ntwt_practise_run(struct ntwt_practise *p)
{
	while (1) {
		usleep(4000);
		pthread_mutex_lock(&p->done_mutex);
		p->action->funct(&p->can_happen,
				 &p->strength,
				 &p->unsatisfied);
		pthread_mutex_unlock(&p->done_mutex);
	}
}

void ntwt_practise_strength(struct ntwt_practise *p, double amount)
{
	pthread_mutex_lock(&p->done_mutex);
	p->strength = amount;
	pthread_mutex_unlock(&p->done_mutex);
}

void ntwt_practise_can_happen(struct ntwt_practise *p, double amount)
{
	pthread_mutex_lock(&p->done_mutex);
	p->can_happen = amount;
	pthread_mutex_unlock(&p->done_mutex);
}

void ntwt_practise_unsatisfied(struct ntwt_practise *p, double amount)
{
	pthread_mutex_lock(&p->done_mutex);
	p->unsatisfied = amount;
	pthread_mutex_unlock(&p->done_mutex);
}

void ntwt_practise_stronger(struct ntwt_practise *p, double amount)
{
	pthread_mutex_lock(&p->done_mutex);
	p->strength += amount * (1 - p->strength);
	pthread_mutex_unlock(&p->done_mutex);
}
