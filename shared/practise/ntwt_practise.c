#include "ntwt_practise.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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
	[0] = { .package_num = 0,
		.id = 0,
		.name = "test",
		.funct = test_fnct }
};

struct ntwt_package ntwt_std_package = {
	.location = NULL,
	.action_num = sizeof(ntwt_built_in_actions) /
	              sizeof(struct ntwt_action),
	.action_max = sizeof(ntwt_built_in_actions) /
	              sizeof(struct ntwt_action),
	.actions = ntwt_built_in_actions
};

struct ntwt_practise *ntwt_practise_new(struct ntwt_action *action,
					double can_happen,
					double strength,
					double unsatisfied)
{
	struct ntwt_practise *p;

	p = malloc(sizeof(struct ntwt_practise));
	p->action = action;
	p->can_happen = can_happen;
	p->strength = strength;
	p->unsatisfied = unsatisfied;
	pthread_mutex_init(&p->done_mutex, NULL);

	return p;
}

struct ntwt_action *ntwt_action_new(char *name,
				    unsigned int package_num,
				    unsigned int id,
				    void (*funct)(double *,
						  double *,
						  double *))
{
	struct ntwt_action *action;

	action = malloc(sizeof(struct ntwt_action));
	action->name = name;
	action->package_num = package_num;
	action->id = id;
	action->funct = funct;

	return action;
}

void ntwt_practise_load(struct ntwt_practise *p,
			struct ntwt_action *action,
			double can_happen,
			double strength,
			double unsatisfied)
{
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
