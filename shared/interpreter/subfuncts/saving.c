
static void save_packages_actions(struct ntwt_package *pack, FILE *image)
{
	struct ntwt_action *action;
	static const char load_action_op[1] = { NTWT_OP_LOAD_ACTION };
	unsigned int j;

	for (j = 0; j != pack->action_max; ++j) {
		action = pack->actions + j;
		if (!action->loaded)
			continue;
		unsigned int action_args[2] = {
			[0] = action->package_num,
			[1] = action->id
		};
		fwrite(load_action_op, sizeof(char), 1, image);
		fwrite(action_args, sizeof(unsigned int),
		       2, image);
		fwrite(action->name, sizeof(char),
		       u8_strlen(action->name) + 1, image);
	}

}

static void save_packages(struct ntwt_instance *state, FILE *image)
{
	struct ntwt_package *pack;
	static const char init_pack_op[1] = { NTWT_OP_INIT_PACK };
	static const char load_pack_op[1] = { NTWT_OP_LOAD_PACK };
	unsigned int i;

	/* Writes packages and their actions */
	fwrite(init_pack_op, sizeof(char), 1, image);
	fwrite(&state->package_max,
	       sizeof(unsigned int), 1, image);
	for (i = 1; i != state->package_max; ++i) {
		pack = state->packages + i;
		if (!pack->loaded)
			continue;
		unsigned int pack_args[2] = {
			[0] = pack->package_num,
			[1] = pack->action_max
		};
		fwrite(load_pack_op, sizeof(char), 1, image);
		fwrite(pack_args, sizeof(unsigned int), 2, image);
		fwrite(pack->location, sizeof(char),
		       u8_strlen(pack->location) + 1, image);
		save_packages_actions(pack, image);
	}
}

static void save_practises(struct ntwt_instance *state, FILE *image)
{
	/* Writes practises and gives them actions */
	struct ntwt_practise *prac;
	unsigned int i;
	static const char load_prac_op[1] = { NTWT_OP_LOAD_PRAC };

	for (i = 0; i != state->practise_max; ++i) {
		printf("hello, %u\n", i);
		prac = state->practises + i;
		if (!prac->loaded)
			continue;
		printf("loaded");
		fwrite(load_prac_op, sizeof(char), 1, image);
		const unsigned int int_args[3] = {
			[0] = i,
			[1] = prac->action->package_num,
			[2] = prac->action->id
		};
		const double double_args[3] = {
			[0] = prac->strength,
			[1] = prac->can_happen,
			[2] = prac->unsatisfied
		};
		fwrite(int_args, sizeof(unsigned int), 3, image);
		fwrite(double_args, sizeof(double), 3, image);
	}
}

static void save(struct ntwt_instance *state)
{
	remove("state.ilk");
	FILE *image = fopen("state.ilk", "ab");
	char c_op[1];

	c_op[0] = NTWT_OP_TEST;
	fwrite(c_op, 1, 1, image);
	save_packages(state, image);
	save_practises(state, image);

	/* c_op[0] = NTWT_OP_TEST; */
	/* fwrite(c_op, 1, 1, image); */
	c_op[0] = NTWT_OP_CONTEXT;
	fwrite(&c_op[0], 1, 1, image);
	c_op[0] = 0;
	fwrite(c_op, 1, 1, image);
	/* c_op[0] = NTWT_OP_ACTION; */
	/* fwrite(&c_op[0], 1, 1, image); */
	/* c_op[0] = NTWT_OP_STRENGTH; */
	/* fwrite(c_op, sizeof(char), 1, image); */
	/* d_op[0] = state->context->strength; */
	/* fwrite(d_op, sizeof(double), 1, image); */
	/* c_op[0] = NTWT_OP_CAN_HAPPEN; */
	/* fwrite(c_op, sizeof(char), 1, image); */
	/* d_op[0] = state->context->can_happen; */
	/* fwrite(d_op, sizeof(double), 1, image); */
	/* c_op[0] = NTWT_OP_UNSATISFIED; */
	/* fwrite(c_op, sizeof(char), 1, image); */
	/* d_op[0] = state->context->unsatisfied; */
	/* fwrite(d_op, sizeof(double), 1, image); */
	c_op[0] = NTWT_OP_RUN;
	fwrite(c_op, 1, 1, image);
	c_op[0] = NTWT_OP_END;
	fwrite(c_op, 1, 1, image);
	fclose(image);
}
