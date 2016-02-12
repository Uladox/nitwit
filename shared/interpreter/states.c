STATE (read) {
	NEXTSTATE(exec_ptr);
}

STATE (end) {
	return;
}

STATE (context) {
	++exec_ptr;
	state->context = state->practises + *exec_ptr;
	NEXTSTATE(exec_ptr);
}

STATE (test) {
	printf("this is a test\n");
	NEXTSTATE(exec_ptr);
}

STATE (awake) {
	pthread_create(&state->awareness, NULL,
		       threaded_awareness_run, NULL);
	NEXTSTATE(exec_ptr);
}

STATE (init_prac) {
	++exec_ptr;
	/* unsigned int */
	POPSET(state->practise_max, exec_ptr);

	state->practises = calloc(state->practise_max,
				  sizeof(struct ntwt_practise));
	POINTEDSTATE(exec_ptr);
}

/* practise_id, action_package_location, action_id,
   strength, can_happen, unsatisfied */
STATE (load_prac) {
	unsigned int practise_id, action_package_location, action_id;
	double strength, can_happen, unsatisfied;
	struct ntwt_practise *prac;
	struct ntwt_action *action;

	++exec_ptr;

	POPSET(practise_id,             exec_ptr);
	POPSET(action_package_location, exec_ptr);
	POPSET(action_id,               exec_ptr);
	POPSET(strength,                exec_ptr);
	POPSET(can_happen,              exec_ptr);
	POPSET(unsatisfied,             exec_ptr);

	prac = state->practises + practise_id;
	action = (state->packages + action_package_location)
		->actions + action_id;
	ntwt_practise_load(prac, action, can_happen, strength,
			   unsatisfied);
	POINTEDSTATE(exec_ptr);
}

STATE (action) {
	unsigned int action_package_location, action_id;

	++exec_ptr;
	POPSET(action_package_location, exec_ptr);
	POPSET(action_id,               exec_ptr);

	state->context->action =
		(state->packages + action_package_location)
		->actions + action_id;
	POINTEDSTATE(exec_ptr);
}

STATE (strength) {
	++exec_ptr;
	ntwt_practise_strength(state->context, POP(double, exec_ptr));
	/* MOVEBY(exec_ptr, double, 1); */
	POINTEDSTATE(exec_ptr);
}

STATE (can_happen) {
	++exec_ptr;
	ntwt_practise_can_happen(state->context, POP(double, exec_ptr));
	/* MOVEBY(exec_ptr, double, 1); */
	POINTEDSTATE(exec_ptr);
}

STATE (unsatisfied) {
	++exec_ptr;
	ntwt_practise_unsatisfied(state->context, POP(double, exec_ptr));
	/* MOVEBY(exec_ptr, double, 1); */
	POINTEDSTATE(exec_ptr);
}

STATE (run) {
	pthread_create(&state->context->thread,
		       NULL, threaded_practise_run,
		       (void *) state->context);
	NEXTSTATE(exec_ptr);
}

STATE (stronger) {
	ntwt_practise_stronger(state->context, 0.1);
	NEXTSTATE(exec_ptr);
}

STATE (save) {
	save(state);
	NEXTSTATE(exec_ptr);
}

/* package_max */
STATE (init_pack) {
	++exec_ptr;

	POPSET(state->package_max, exec_ptr);

	++state->package_max;
	 /* = *((unsigned int *) exec_ptr) + 1; */
	state->packages = calloc
		(state->package_max,
		 sizeof(struct ntwt_package));
	*state->packages = ntwt_std_package;
	state->package_ptr = 1;
	/* MOVEBY(exec_ptr, unsigned int, 1); */
	printf("wot?!\n");

	/* For testing only */
	/* state->practises->can_happen = 0.5; */
	/* state->practises->strength = 0.5; */
	/* state->practises->unsatisfied = 0.5; */
	POINTEDSTATE(exec_ptr);
}

/* package_num, action_max, location, */
STATE (load_pack) {
	unsigned int package_num;
	unsigned int action_max;
	char *location;
	size_t location_size;

	++exec_ptr;

	POPSET(package_num,    exec_ptr);
	POPSET(action_max,     exec_ptr);
	POPSETSTRING(location, exec_ptr);

	ntwt_instance_load_package(state,
				   package_num,
				   action_max,
				   location);

	POINTEDSTATE(exec_ptr);
}

/* package_num, id, action_name */
STATE (load_action) {
	unsigned int package_num, id;
	char *action_name;
	size_t action_name_size;

	++exec_ptr;

	POPSET(package_num,       exec_ptr);
	POPSET(id,                exec_ptr);
	POPSETSTRING(action_name, exec_ptr);

	ntwt_package_load_action(state->packages + package_num,
				 id, action_name);
	POINTEDSTATE(exec_ptr);
}
