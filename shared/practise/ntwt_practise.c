#include "ntwt_practise.h"

#include <stdlib.h>
#include <unistd.h>

struct ntwt_practise *ntwt_practise_new(void (*action)(double *,
					     double *,
					     double *),
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

void ntwt_practise_load(struct ntwt_practise *p,
			void (*action)(double *,
				       double *,
				       double *),
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
		p->action(&p->can_happen,
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
