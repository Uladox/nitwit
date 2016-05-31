#define _DEFAULT_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistr.h>

#define NTWT_SHORT_NAMES
#include "../list/list.h"
#include "../hash/hashmap.h"
#include "../../gen/output/op_map.h"
#include "../macros.h"
#include "../vm/state.h"
#include "../vm/vm.h"
#include "../vm/vm_data.h"
#include "compiler.h"
#include "lex_info.h"
#include "lex.h"

#define lex(...) asm_lex(__VA_ARGS__)

static struct ntwt_asm_expr *
pop(struct ntwt_asm_expr **stack)
{
	if (*stack) {
		struct ntwt_asm_expr *tmp = *stack;
		*stack = NTWT_LIST_NEXT(*stack);
		return tmp;
	}

	return malloc(sizeof(**stack));
}

static void
term(struct ntwt_asm_lex_info *info,
     struct ntwt_asm_expr **stack,
     struct ntwt_asm_expr *cmd)
{
	struct ntwt_asm_expr *trm = pop(stack);

	*info->trms = trm;
	trm->lineno = info->lexme_lineno;
	trm->next = NULL;
	switch (trm->type = info->token) {
	case NTWT_UINT:
		/* treats utf8 as ascii for stroul */
		cmd->size += (trm->size = sizeof(trm->contents.integer));
		trm->contents.integer =
			htobe32(strtoul((char *) info->lexme, NULL, 0));
		break;
	case NTWT_DOUBLE:
		/* treats utf8 as ascii for strod */
		cmd->size += (trm->size = sizeof(trm->contents.decimal));
		trm->contents.decimal = strtod((char *) info->lexme, NULL);
		break;
	case NTWT_STRING:
		cmd->size += (trm->size = info->lexlen + 1);
		trm->contents.string = malloc(trm->size);
		u8_strncpy(trm->contents.string, info->lexme, info->lexlen);
		trm->contents.string[info->lexlen] = '\0';
		break;
	case NTWT_OP_CODE:
		cmd->size += (trm->size = sizeof(trm->contents.op_code));

		char *result = hashmap_get(&ntwt_op_map,
					   info->lexme,
					   info->lexlen);

		if (!result) {
			trm->size = -1;
			*info->error = 1;
			break;
		}

		trm->contents.op_code = *result;
		break;
	default:
		/* Should never happen because lex should only return above */
		fprintf(stderr,
			"Error: unrecognized token on line %u\n",
			info->lineno);
		*info->error = 1;
	}
	info->trms = NTWT_NEXT_REF(trm);
	lex(info);
}

static void
command(struct ntwt_asm_lex_info *info,
	struct ntwt_asm_expr **stack,
	struct ntwt_asm_program *program)
{
	struct ntwt_asm_expr *cmd = pop(stack);

	cmd->type = NTWT_COMMAND;
	cmd->next = NULL;
	cmd->size = 0;
	cmd->contents.list = NULL;
	info->trms = &cmd->contents.list;

	while (info->token != NTWT_SEMICOLON &&
	       info->token != NTWT_EOI)
		term(info, stack, cmd);
	lex(info);

	if (cmd->size == 0) {
		fprintf(stderr,
			"Error: extra semicolon on line %u\n",
			info->lineno);
		goto error;
	}

	if (cmd->contents.list[0].size == -1) {
		fprintf(stderr,
			"Error: unrecognized op code on line %u\n",
			info->lineno);
		goto error;
	}

	if (cmd->contents.list[0].type != NTWT_OP_CODE) {
		fprintf(stderr,
			"Error: command does not start with OP_CODE on line %u\n",
			info->lineno);
		goto error;
	}

	program->size += cmd->size;
	*info->cmds = cmd;
	info->cmds = NTWT_NEXT_REF(cmd);
	return;
error:
	*info->error = 1;
	asm_recycle(stack, cmd);
}

void
asm_statements(struct ntwt_asm_program *program,
	       struct ntwt_asm_expr **stack,
	       const uint8_t *code, int *error)
{
	struct ntwt_asm_lex_info info = {
		.lexme = code,
		.lexlen = 0,
		.units = u8_strlen(code),
		.lineno = 1,
		.token = NTWT_SEMICOLON,
		.cmds = &program->expr,
		.error = error
	};

	program->size = 0;
	program->expr = NULL;

	lex(&info);
	while (info.token != NTWT_EOI)
		command(&info, stack, program);
}

static void
asm_command_type_check(struct ntwt_asm_expr *cmd, int *error)
{
	struct ntwt_asm_expr *term = cmd->contents.list;
	const unsigned int cmd_line = term->lineno;
	const char *cmd_name = ntwt_op_name[(uint8_t) term->contents.op_code];
	const int *params = ntwt_op_args[(uint8_t) term->contents.op_code];

	term = NTWT_LIST_NEXT(term);

	int i = 1;

	ntwt_foreach (term) {
		++i;
		if (i > params[0]) {
			fprintf(stderr,
				"Error: too many arguments to %s on line %u, expected %u, got more\n",
			        cmd_name,
			        cmd_line, params[0]);
			*error = 1;
			return;
		}

		if (params[i] != term->type) {
			fprintf(stderr,
				"Error: on line %u expected type %s, got %s\n",
				term->lineno, ntwt_type_name[params[i]],
				ntwt_type_name[term->type]);
			*error = 1;
		}
	}

	if (i - 1 < params[0]) {
		fprintf(stderr,
			"Error: too few arguments to %s on line %u, expected %u, got %u\n",
		        cmd_name, cmd_line,
			params[0], i - 1);
		*error = 1;
	}
}

void
asm_program_type_check(struct ntwt_asm_program *program, int *error)
{
	struct ntwt_asm_expr *cmd = program->expr;

	ntwt_foreach (cmd)
		asm_command_type_check(cmd, error);
}

static void asm_term_bytecode(struct ntwt_asm_expr *term, char **code_ptr,
			      int *error)
{
	switch (term->type) {
	case NTWT_UINT:
		memcpy(*code_ptr, &term->contents.integer, term->size);
		break;
	case NTWT_DOUBLE:
		memcpy(*code_ptr, &term->contents.decimal, term->size);
		break;
	case NTWT_STRING:
		memcpy(*code_ptr, term->contents.string, term->size);
		break;
	case NTWT_OP_CODE:
		memcpy(*code_ptr, &term->contents.op_code, term->size);
		break;
	default:
		fprintf(stderr,
			"Error: Unrecognized type in asm tree to bytecode, %u.\n",
			term->type);
		*error = 1;
	}
	*code_ptr += term->size;
}

static void
asm_command_bytecode(struct ntwt_asm_expr *cmd, char **code_ptr, int *error)
{
	struct ntwt_asm_expr *term = cmd->contents.list;

	ntwt_foreach (term)
		asm_term_bytecode(term, code_ptr, error);
}

void
asm_program_bytecode(struct ntwt_asm_program *program,
		     char **code, size_t *old_size,
		     unsigned int *message_size, int *error)
{
	*message_size = program->size;
	if (unlikely(program->size > *old_size)) {
		free(*code);
		*code = malloc(program->size);
		*old_size = program->size;
	}

	char *code_ptr = *code;
	struct ntwt_asm_expr *cmd = program->expr;

	ntwt_foreach (cmd)
		asm_command_bytecode(cmd, &code_ptr, error);
}

void
asm_recycle(struct ntwt_asm_expr **stack, struct ntwt_asm_expr *expr)
{
	while (expr) {
		struct ntwt_asm_expr *tmp;

		if (expr->type == NTWT_STRING)
			free(expr->contents.string);
		else if (expr->type == NTWT_COMMAND)
			asm_recycle(stack, expr->contents.list);

		tmp = expr;
		expr = NTWT_LIST_NEXT(expr);
		NTWT_LIST_CONS(tmp, *stack);
		*stack = tmp;
	}
}

void
asm_expr_free(struct ntwt_asm_expr *expr)
{
	while (expr) {
		struct ntwt_asm_expr *tmp;

		if (expr->type == NTWT_STRING)
			free(expr->contents.string);
		else if (expr->type == NTWT_COMMAND)
			asm_expr_free(expr->contents.list);

		tmp = expr;
		expr = NTWT_LIST_NEXT(expr);
		free(tmp);
	}
}

void
asm_stack_free(struct ntwt_asm_expr *stack)
{
	while (stack) {
		struct ntwt_asm_expr *tmp;

		tmp = stack;
		stack = NTWT_LIST_NEXT(stack);
		free(tmp);
	}
}
