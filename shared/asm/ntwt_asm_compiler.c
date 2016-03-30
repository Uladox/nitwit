#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistr.h>

#define NTWT_SHORT_NAMES
#include "ntwt_asm_compiler.h"
#include "../nitwit_macros.h"
#include "../interpreter/ntwt_interpreter.h"
#include "../../gen/output/ntwt_op_map.h"

struct lex_info {
	const uint8_t *lexme;
	size_t lexlen;
	size_t units;
	unsigned int lineno;
	unsigned int token;
	unsigned int offset;
};

static struct ntwt_asm_expr *command(struct lex_info *info,
				struct ntwt_asm_expr **stack);

static struct ntwt_asm_expr *term(struct lex_info *info,
				  struct ntwt_asm_expr **stack);

static void lex_string(struct lex_info *info, const uint8_t *current)
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
			exit(1);
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

static void lex_num(struct lex_info *info, const uint8_t *current)
{
	unsigned int num_type = NTWT_UINT;
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
			if (likely(!period))
				break;
			fprintf(stderr,
				"Error: to many '.' in number on line %u\n",
				info->lineno);
			exit(1);
		case '\0':
			fprintf(stderr,
				"Error: unexpected end of input on line %u\n",
				info->lineno);
			exit(1);
		default:
			fprintf(stderr,
				"Error: invalid char '%c' in number on line %u\n",
				*current, info->lineno);
			exit(1);
		}
	}
}

static void lex_op_code(struct lex_info *info, const uint8_t *current)
{
	while (!isspace(*current) && *current != ';') {
		++current;
		--info->units;
	}

	info->lexlen = current - info->lexme;
	info->token = NTWT_OP_CODE;
}

static void lex(struct lex_info *info)
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
		lex_string(info, current);
		break;
	case '0' ... '9':
		lex_num(info, current);
		break;
	case '*':
		/* Assumes an op code after '*' */
		++info->lexme;
		lex_op_code(info, current);
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
			exit(1);
		}
		lex_op_code(info, current);
		break;
	}
}


static inline int match(struct lex_info *info, unsigned int token)
{
	return info->token == token;
}

static inline void advance(struct lex_info *info)
{
	lex(info);
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

void asm_statements(struct ntwt_asm_program *program,
		    struct ntwt_asm_expr **stack,
		    const uint8_t *code)
{
	struct ntwt_asm_expr *expr;
	struct lex_info info = {
		.lexme = code,
		.lexlen = 0,
		.units = u8_strlen(code),
		.lineno = 0,
		.token = NTWT_SEMICOLON
	};

	program->size = 0;

	advance(&info);
	expr = (program->expr = command(&info, stack));

	if (unlikely(!match(&info, NTWT_SEMICOLON))) {
		fprintf(stderr,
			"Error: Inserting missing semicolon on line %u\n",
			info.lineno);
		exit(1);
	}

	program->size += expr->size;
	advance(&info);
	while (!match(&info, NTWT_EOI)) {
		expr = (expr->next = command(&info, stack));
		if (unlikely(!match(&info, NTWT_SEMICOLON))) {
			fprintf(stderr,
				"Error: Insert missing semicolon on line %u\n",
				info.lineno);
			exit(1);
		}
		program->size += expr->size;
		advance(&info);
	}
}

static struct ntwt_asm_expr *command(struct lex_info *info,
				     struct ntwt_asm_expr **stack)
{
	struct ntwt_asm_expr *expr = pop(stack);
	struct ntwt_asm_expr *list;

	expr->type = NTWT_COMMAND;
	expr->next = NULL;
	expr->size = 0;

	list = (expr->contents.list = term(info, stack));
	expr->size += list->size;
	advance(info);
	while (!match(info, NTWT_SEMICOLON)) {
		if (unlikely(match(info, NTWT_EOI))) {
			fprintf(stderr,
				"Error: end of input on line: %u\n",
				info->lineno);
			exit(1);
		}
		list = (list->next = term(info, stack));
		expr->size += list->size;
		advance(info);
	}

	return expr;
}

static struct ntwt_asm_expr *term(struct lex_info *info,
				  struct ntwt_asm_expr **stack)
{
	struct ntwt_asm_expr *expr = pop(stack);

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
			exit(1);
		}

		expr->contents.op_code = *result;
		break;
	default:
		fprintf(stderr,
			"Error: unrecognized token on line %u\n",
			info->lineno);
		exit(1);
	}

	return expr;
}

static void asm_term_bytecode(struct ntwt_asm_expr *term, char **code_ptr)
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
	}
	*code_ptr += term->size;
}

static void asm_command_bytecode(struct ntwt_asm_expr *command, char **code_ptr)
{
	struct ntwt_asm_expr *term = command->contents.list;

	for (; term; term = term->next)
	        asm_term_bytecode(term, code_ptr);
}

void asm_program_bytecode(struct ntwt_asm_program *program,
			  char **code, size_t *old_size,
			  unsigned int *message_size)
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
	        asm_command_bytecode(command, &code_ptr);
}

void asm_recycle(struct ntwt_asm_expr **stack, struct ntwt_asm_expr *top)
{
	struct ntwt_asm_expr *expr = top;

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
