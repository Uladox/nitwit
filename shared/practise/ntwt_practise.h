#ifndef NTWT_PRACTISE_H
#define NTWT_PRACTISE_H

#include <stdint.h>
#include <pthread.h>

struct ntwt_instance;

struct ntwt_action {
	char loaded;
	uint32_t package_num;
	uint32_t id;
	uint8_t *name;
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
	uint32_t package_num;

	uint8_t *location;
	void *handle;
	uint32_t action_ptr;
	uint32_t action_max;
	struct ntwt_action *actions;
};

extern struct ntwt_package ntwt_std_package;

struct ntwt_action *ntwt_action_new(uint8_t *name,
				    uint32_t package_num,
				    uint32_t id,
				    void (*funct)(double *,
						  double *,
						  double *));

void ntwt_practise_load(struct ntwt_practise *p,
			struct ntwt_action *action,
			double can_happen,
			double strength,
			double unsatisfied);

void ntwt_instance_load_package(struct ntwt_instance *instance,
				uint32_t package_num,
				uint32_t action_max,
				uint8_t *location);

void ntwt_package_load_action(struct ntwt_package *package,
			      uint32_t id,
			      uint8_t *action_name);

void ntwt_practise_run(struct ntwt_practise *p);

void ntwt_practise_strength(struct ntwt_practise *p, double amount);

void ntwt_practise_can_happen(struct ntwt_practise *p, double amount);

void ntwt_practise_unsatisfied(struct ntwt_practise *p, double amount);

void ntwt_practise_stronger(struct ntwt_practise *p, double amount);

#endif
