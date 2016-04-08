#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>
#include <unistr.h>

#define NTWT_SHORT_NAMES
#include "asm_compiler.h"
#include "../vm/vm_data.h"
#include "../nitwit_macros.h"
#include "../../gen/output/op_map.h"

enum { ERROR_EOI=1, ERROR_SEMI };

struct lex_info {
	const uint8_t *lexme;
	enum ntwt_token token;
	size_t lexlen;
	size_t units;
	unsigned int lexme_lineno;
	unsigned int lineno;
	unsigned int offset;
	jmp_buf err_jmp;
};

static void lex_string(struct lex_info *info, const uint8_t *current,
		       int *error)
{
	int backslashed = 0;
	while (1) {
		++current;
		--info->units;
		switch (*current) {
		case '\0':
			fprintf(stderr,
				"Error: string has no ending quote on line %u\n",
				info->lineno);
			longjmp(info->err_jmp, ERROR_EOI);
		case '\\':
			backslashed = !backslashed;
			break;
		case '\n':
			++info->lineno; backslashed = 0;
			break;
		case '\"':
			if (backslashed)
				break;
			info->lexlen = current - info->lexme;
			info->offset = 1;
			info->token =  NTWT_STRING;
			return;
		default:
			backslashed = 0;
			break;
		}
	}
}

static void lex_num(struct lex_info *info, const uint8_t *current,
		    int *error)
{
	enum ntwt_token num_type = NTWT_UINT;
	int period = 0;

	while (1) {
		++current;
		--info->units;
		switch (*current) {
		case '0' ... '9':
			continue;
		case '\n':
			++info->lineno;
		case ' ':
		case '\t':
		case '\v':
		case '\f':
		case '\r':
		case ';':
			info->lexlen = current - info->lexme;
			info->token = num_type;
			return;
		case '.':
			num_type = NTWT_DOUBLE;
			if (likely(!period)) {
				period = 1;
				break;
			}
			fprintf(stderr,
				"Error: to many '.' in number on line %u\n",
				info->lineno);
			*error = 1;
			break;
		case '\0':
			fprintf(stderr,
				"Error: unexpected end of input on line %u\n",
				info->lineno);
			longjmp(info->err_jmp, ERROR_EOI);
		default:
			fprintf(stderr,
				"Error: invalid char '%c' in number on line %u\n",
				*current, info->lineno);
			*error = 1;
			break;
		}
	}
}

static void lex_op_code(struct lex_info *info, const uint8_t *current,
			int *error)
{
	while (!isspace(*current) && *current != ';') {
		if (*current == '\0') {
			fprintf(stderr,
				"Error: unexpected end of input on line %u\n",
				info->lineno);
			longjmp(info->err_jmp, ERROR_EOI);
		}
		++current;
		--info->units;
	}
	info->lexlen = current - info->lexme;
	info->token = NTWT_OP_CODE;
}

static void lex(struct lex_info *info, int *error)
{
	const uint8_t *current = info->lexme;
	ucs4_t puc;

	current += info->lexlen + info->offset;
	info->units -= info->lexlen + info->offset;
	while (isspace(*current)) {
		if (*current == '\n')
			++info->lineno;
		++current;
		--info->units;
	}

	info->lexlen = 1;
	info->offset = 0;
	info->lexme = current;
	info->lexme_lineno = info->lineno;

	u8_mbtouc_unsafe(&puc, current, info->units);
	switch (puc) {
	case '\0':
		info->token = NTWT_EOI;
		break;
	case ';':
		info->token = NTWT_SEMICOLON;
		break;
	case '"':
		++info->lexme;
		lex_string(info, current, error);
		break;
	case '0' ... '9':
		lex_num(info, current, error);
		break;
	case '*':
		/* Assumes an op code after '*' */
		++info->lexme;
		lex_op_code(info, current, error);
		break;
	case U'🐣':
		printf(u8"λ🐣!\n");
		exit(0);
	default:
		/* Assumes an  op code after ';' */
		if (info->token != NTWT_SEMICOLON) {
			fprintf(stderr,
				"Error: invalid argument on line %u, if you want an op_code, put '*' before it.\n",
				info->lineno);
			*error = 1;
		}
		lex_op_code(info, current, error);
		break;
	}
}

static struct ntwt_asm_expr *pop(struct ntwt_asm_expr **stack)
{
	if (*stack) {
		struct ntwt_asm_expr *tmp = *stack;
		*stack = (*stack)->next;
		return tmp;
	}
	return malloc(sizeof(**stack));
}

static void term(struct lex_info *info,
		 struct ntwt_asm_expr **load_expr,
		 struct ntwt_asm_expr **stack,
		 int *error)
{
	struct ntwt_asm_expr *expr = (*load_expr = pop(stack));

	expr->lineno = info->lexme_lineno;
	expr->next = NULL;
	switch (expr->type = info->token) {
	case NTWT_UINT:
		expr->size = sizeof(unsigned int);
		/* treats utf8 as ascii for stroul */
		expr->contents.integer = strtoul((char *) info->lexme, NULL, 0);
		break;
	case NTWT_DOUBLE:
		expr->size = sizeof(double);
		/* treats utf8 as ascii for strod */
		expr->contents.decimal = strtod((char *) info->lexme, NULL);
		break;
	case NTWT_STRING:
		expr->contents.string = malloc(expr->size = info->lexlen + 1);
		u8_strncpy(expr->contents.string, info->lexme, info->lexlen);
		expr->contents.string[info->lexlen] = '\0';
		break;
	case NTWT_OP_CODE:
		expr->size = sizeof(char);

		char *result = ntwt_hashmap_get(&ntwt_op_map, info->lexme,
						info->lexlen);
		if (!result) {
			fprintf(stderr,
				"Error: unrecognized op code on line %u\n",
				info->lineno);
			*error = 1;
			return;
		}
		expr->contents.op_code = *result;
		break;
	case NTWT_SEMICOLON:
		fprintf(stderr,
			"Error: extra semicolon on line %u\n",
			info->lineno);
		asm_recycle(stack, expr);
		longjmp(info->err_jmp, ERROR_SEMI);
	case NTWT_EOI:
		fprintf(stderr,
			"Error: unexpected end of input on line %u\n",
			info->lineno);
		asm_recycle(stack, expr);
		longjmp(info->err_jmp, ERROR_EOI);
	default:
		/* Should never happen because lex should only return above */
		fprintf(stderr,
			"Error: unrecognized token on line %u\n",
			info->lineno);
		*error = 1;
	}
}

static void command(struct lex_info *info,
		    struct ntwt_asm_expr **load_expr,
		    struct ntwt_asm_expr **stack,
		    int *error)
{
	struct ntwt_asm_expr *cmd;
	struct ntwt_asm_expr *expr;

	*load_expr = NULL;

	term(info, &expr, stack, error);

	cmd = (*load_expr = pop(stack));
	cmd->type = NTWT_COMMAND;
	cmd->next = NULL;
	cmd->contents.list = expr;
	cmd->size = expr->size;

	lex(info, error);
	while (info->token != NTWT_SEMICOLON) {
	        term(info, &expr->next, stack, error);
		expr = expr->next;
		cmd->size += expr->size;
		lex(info, error);
	}
}

void asm_statements(struct ntwt_asm_program *program,
		    struct ntwt_asm_expr **stack,
		    const uint8_t *code, int *error)
{
	struct ntwt_asm_expr *expr;
	struct lex_info info = {
		.lexme = code,
		.lexlen = 0,
		.units = u8_strlen(code),
		.lineno = 0,
		.token = NTWT_SEMICOLON
	};

	program->expr = NULL;
	program->size = 0;

	*error = setjmp(info.err_jmp);
	if (*error) {
		if (*error == ERROR_EOI)
			return;
		else if (*error == ERROR_SEMI)
			goto next_command;

	}


	lex(&info, error);
	command(&info, &program->expr, stack, error);
	expr = program->expr;
	program->size += expr->size;

next_command:

	lex(&info, error);
	while (info.token != NTWT_EOI) {
		command(&info, &expr->next, stack, error);
		expr = expr->next;
		program->size += expr->size;
		lex(&info, error);
	}
}

static void asm_command_type_check(struct ntwt_asm_expr *command, int *error)
{
	struct ntwt_asm_expr *term = command->contents.list;
	const unsigned int command_line = term->lineno;
	const char *command_name =
		ntwt_op_name[(uint8_t) term->contents.op_code];
	const int *params = ntwt_op_args[(uint8_t) term->contents.op_code];

	term = term->next;

	int i = 1;
	for (; term; term = term->next, ++i) {
		if (i > params[0]) {
			fprintf(stderr,
				"Error: too many arguments to %s on line %u, "
				"expected %u, got more\n", command_name,
				command_line, params[0]);
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
			"Error: too few arguments to %s on line %u, "
			"expected %u, got %u\n", command_name, command_line,
			params[0], i - 1);
		*error = 1;
	}
}

void asm_program_type_check(struct ntwt_asm_program *program,
			    int *error)
{
	struct ntwt_asm_expr *command;

	for (command = program->expr;
	     command; command = command->next)
	        asm_command_type_check(command, error);
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

static void asm_command_bytecode(struct ntwt_asm_expr *command, char **code_ptr,
				 int *error)
{
	struct ntwt_asm_expr *term = command->contents.list;

	for (; term; term = term->next)
	        asm_term_bytecode(term, code_ptr, error);
}

void asm_program_bytecode(struct ntwt_asm_program *program,
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
	struct ntwt_asm_expr *command;

	for (command = program->expr;
	     command; command = command->next)
	        asm_command_bytecode(command, &code_ptr, error);
}

void asm_recycle(struct ntwt_asm_expr **stack, struct ntwt_asm_expr *expr)
{
	while (expr) {
		struct ntwt_asm_expr *tmp;

		if (expr->type == NTWT_STRING)
			free(expr->contents.string);
		else if (expr->type == NTWT_COMMAND)
			asm_recycle(stack, expr->contents.list);
		tmp = expr;
		expr = expr->next;
		tmp->next = *stack;
		*stack = tmp;
	}
}

void asm_expr_free(struct ntwt_asm_expr *expr)
{
	while (expr) {
		struct ntwt_asm_expr *tmp;

		if (expr->type == NTWT_STRING)
			free(expr->contents.string);
		else if (expr->type == NTWT_COMMAND)
			asm_expr_free(expr->contents.list);
		tmp = expr;
		expr = expr->next;
		free(tmp);
	}
}

void asm_stack_free(struct ntwt_asm_expr *stack)
{
	while (stack) {
		struct ntwt_asm_expr *tmp;

		tmp = stack;
		stack = stack->next;
		free(tmp);
	}
}
