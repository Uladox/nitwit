#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistr.h>

#define NTWT_SHORT_NAMES
#include "../macros.h"
#include "../vm/state.h"
#include "../vm/vm.h"
#include "../vm/vm_data.h"
#include "lex_info.h"
#include "lex.h"

static void
lex_string(struct ntwt_asm_lex_info *info, const uint8_t *current)
{
	int backslashed = 0;

	info->token =  NTWT_STRING;
	while (1) {
		++current;
		--info->units;
		switch (*current) {
		case '\0':
			fprintf(stderr,
				"Error: string has no ending quote on line %u\n",
				info->lineno);
			info->lexlen = current - info->lexme;
			*info->error = 1;
			return;
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
			return;
		default:
			backslashed = 0;
			break;
		}
	}
}

static void
lex_num(struct ntwt_asm_lex_info *info, const uint8_t *current)
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
			*info->error = 1;
			break;
		case '\0':
			fprintf(stderr,
				"Error: unexpected end of input on line %u\n",
				info->lineno);
			*info->error = 1;
			info->lexlen = current - info->lexme;
			info->token = num_type;
			return;
		default:
			fprintf(stderr,
				"Error: invalid char '%c' in number on line %u\n",
				*current, info->lineno);
			*info->error = 1;
			break;
		}
	}
}

static void
lex_op_code(struct ntwt_asm_lex_info *info, const uint8_t *current)
{
	while (!isspace(*current) && *current != ';' && *current != '\0') {
		++current;
		--info->units;
	}
	if (*current == '\0') {
		fprintf(stderr,
			"Error: unexpected end of input on line %u\n",
			info->lineno);
		*info->error = 1;
	}
	info->lexlen = current - info->lexme;
	info->token = NTWT_OP_CODE;
}

void
ntwt_asm_lex(struct ntwt_asm_lex_info *info)
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

	info->offset = 0;
	info->lexme = current;
	info->lexme_lineno = info->lineno;

	u8_mbtouc_unsafe(&puc, current, info->units);
	switch (puc) {
	case '\0':
		info->lexlen = 0;
		info->token = NTWT_EOI;
		break;
	case ';':
		info->lexlen = 1;
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
			*info->error = 1;
		}

		lex_op_code(info, current);
		break;
	}
}
