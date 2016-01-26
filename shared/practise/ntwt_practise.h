#ifndef NTWT_PRACTISE_H
#define NTWT_PRACTISE_H

#include <pthread.h>

struct ntwt_practise {
	void (*action)(double *,
		       double *,
		       double *);
	double can_happen;
	double strength;
	double unsatisfied;

	pthread_t thread;
	pthread_mutex_t done_mutex;
};

void ntwt_practise_load(struct ntwt_practise *p,
			void (*action)(double *,
				       double *,
				       double *),
			double can_happen,
			double strength,
			double unsatisfied);

void ntwt_practise_run(struct ntwt_practise *p);

void ntwt_practise_stronger(struct ntwt_practise *p, double amount);

#endif
