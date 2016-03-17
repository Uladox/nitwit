STATE (read) {
	printf("This should never be read!\n");
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

#if ASSUME_UTF8
STATE(echo) {
	++exec_ptr;
	printf("%s\n", exec_ptr);
	exec_ptr += u8_strlen((uint8_t *) exec_ptr) + 1;
	POINTEDSTATE(exec_ptr);
}
#else
#warning "Less efficient if your locale is utf8."
STATE (echo) {
	uint8_t *uni_str;
	char *io;
	size_t uni_size;

	++exec_ptr;
	POPSETSTRING(uni_str, uni_size, exec_ptr);
	printf("%s\n", io = u8_strconv_to_locale(uni_str));
	free(io);
	free(uni_str);
	POINTEDSTATE(exec_ptr);
}
#endif

STATE (awake) {
	pthread_create(&state->awareness, NULL,
		       threaded_awareness_run, NULL);
	NEXTSTATE(exec_ptr);
}

STATE (init_prac) {
	++exec_ptr;
	POPSET(state->practise_max, exec_ptr);

	state->practises = calloc(state->practise_max,
				  sizeof(*state->practises));
	POINTEDSTATE(exec_ptr);
}

STATE (load_prac) {
	uint32_t prac_id;
	uint32_t pkg_pos;
	uint32_t action_id;
	double can_happen;
	double strength;
	double unsatisfied;
	struct ntwt_practise *prac;
	struct ntwt_action *action;

	++exec_ptr;

	POPSET(prac_id,      exec_ptr);
	POPSET(pkg_pos,       exec_ptr);
	POPSET(action_id,    exec_ptr);
	POPSET(can_happen,   exec_ptr);
	POPSET(strength,     exec_ptr);
	POPSET(unsatisfied,  exec_ptr);

	prac = state->practises + prac_id;
	action = (state->packages + pkg_pos)->actions + action_id;
	ntwt_practise_load(prac, action, can_happen, strength, unsatisfied);
	POINTEDSTATE(exec_ptr);
}

STATE (action) {
	uint32_t pkg_pos;
	uint32_t id;

	++exec_ptr;
	POPSET(pkg_pos, exec_ptr);
	POPSET(id,           exec_ptr);

	state->context->action =
		(state->packages + pkg_pos)->actions + id;
	POINTEDSTATE(exec_ptr);
}


STATE (can_happen) {
	++exec_ptr;
	ntwt_practise_can_happen(state->context, POP(double, exec_ptr));
	POINTEDSTATE(exec_ptr);
}

STATE (strength) {
	++exec_ptr;
	ntwt_practise_strength(state->context, POP(double, exec_ptr));
	POINTEDSTATE(exec_ptr);
}

STATE (unsatisfied) {
	++exec_ptr;
	ntwt_practise_unsatisfied(state->context, POP(double, exec_ptr));
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

STATE (init_pack) {
	++exec_ptr;

	POPSET(state->package_max, exec_ptr);

	++state->package_max;
	state->packages = calloc
		(state->package_max,
		 sizeof(*state->packages));
	*state->packages = ntwt_std_package;
	state->package_ptr = 1;

	/* For testing only */
	/* state->practises->can_happen = 0.5; */
	/* state->practises->strength = 0.5; */
	/* state->practises->unsatisfied = 0.5; */
	POINTEDSTATE(exec_ptr);
}

STATE (load_pack) {
	uint32_t pkg_num;
	uint32_t action_max;
	uint8_t *path;
	size_t path_size;

	++exec_ptr;

	POPSET(pkg_num,               exec_ptr);
	POPSET(action_max,            exec_ptr);
	POPSETSTRING(path, path_size, exec_ptr);

	ntwt_instance_load_package(state, pkg_num, action_max, path);

	POINTEDSTATE(exec_ptr);
}

STATE (load_action) {
	uint32_t pkg_num;
	uint32_t id;
	uint8_t *name;
	size_t name_size;

	++exec_ptr;

	POPSET(pkg_num,               exec_ptr);
	POPSET(id,                    exec_ptr);
	POPSETSTRING(name, name_size, exec_ptr);

	ntwt_package_load_action(state->packages + pkg_num, id, name);
	POINTEDSTATE(exec_ptr);
}
