#ifndef NTWT_PRACTISE_H
#define NTWT_PRACTISE_H

#include <pthread.h>

struct ntwt_instance;

struct ntwt_action {
	char loaded;
	unsigned int package_num;
	unsigned int id;
	char *name;
	void (*funct)(double *,
		      double *,
		      double *);

};

struct ntwt_practise {
	char loaded;
	struct ntwt_action *action;
	double can_happen;
	double strength;
	double unsatisfied;

	pthread_t thread;
	pthread_mutex_t done_mutex;
};

struct ntwt_package {
	char loaded;
	unsigned int package_num;

	char *location;
	void *handle;
	unsigned int action_ptr;
	unsigned int action_max;
	struct ntwt_action *actions;
};

extern struct ntwt_package ntwt_std_package;

struct ntwt_action *ntwt_action_new(char *name,
				    unsigned int package_num,
				    unsigned int id,
				    void (*funct)(double *,
						  double *,
						  double *));

void ntwt_practise_load(struct ntwt_practise *p,
			struct ntwt_action *action,
			double can_happen,
			double strength,
			double unsatisfied);

void ntwt_instance_load_package(struct ntwt_instance *instance,
				unsigned int package_num,
				unsigned int action_max,
				char *location);

void ntwt_package_load_action(struct ntwt_package *package,
			      unsigned int id,
			      char *action_name);

void ntwt_practise_run(struct ntwt_practise *p);

void ntwt_practise_strength(struct ntwt_practise *p, double amount);

void ntwt_practise_can_happen(struct ntwt_practise *p, double amount);

void ntwt_practise_unsatisfied(struct ntwt_practise *p, double amount);

void ntwt_practise_stronger(struct ntwt_practise *p, double amount);

#endif
