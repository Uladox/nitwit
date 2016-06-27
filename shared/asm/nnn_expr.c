#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include <nitlib/list.h>
#include <nitlib/hashmap.h>

#include <spar/core.h>
#include <spar/text_utils.h>
#include <spar/token_print.h>
#include <spar/cusstrlit.h>
#include <spar/cusword.h>
#include <spar/cusnum.h>

#include "../../gen/output/op_map.h"

#include "../vm/vm.h"
#include "nnn_parser.h"
#include "nnn_expr.h"

struct nnn_expr *
nnn_prog_pop(struct nnn_prog *prog);

static inline void
nnn_set_op_code(struct nnn_prog *prog, struct nnn_expr *expr,
		struct spar_token *token)
{
	const char *result;

	prog->size += (expr->size = sizeof(expr->dat.op_code));
	result = hashmap_get(&ntwt_op_map, token->dat.text, token->len);

	if (!result) {
		prog->parsed = 0;
		fprintf(stderr, "Error: line %zu, invalid op code \"",
			expr->line);
		spar_fprint_text_token(token, stderr);
		fprintf(stderr, "\"\n");
		expr->dat.op_code = NTWT_OP_INVALID;
		return;
	}

	expr->dat.op_code = *result;
}


/* Fix this! */
static inline void
nnn_set_string(struct nnn_prog *prog, struct nnn_expr *expr,
	       struct spar_token *token)
{
	prog->size += (expr->size = token->data_size);

	if (!spar_strlit_str(token, &expr->dat.string)) {
		prog->parsed = 0;
		fprintf(stderr,
			"Error: on line %zu, invalid string ",
			expr->line);
		spar_fprint_text_token(token, stderr);
		printf("\n");
	}
}

static inline void
nnn_set_uint(struct nnn_prog *prog, struct nnn_expr *expr,
	     struct spar_token *token)
{
	prog->size += (expr->size = sizeof(expr->dat.integer));
	expr->dat.integer = strtoul(token->dat.text, NULL, 0);
}

static inline void
nnn_set_double(struct nnn_prog *prog, struct nnn_expr *expr,
	       struct spar_token *token)
{
	prog->size += (expr->size = sizeof(expr->dat.decimal));
	expr->dat.integer = strtod(token->dat.text, NULL);
}


struct nnn_expr *
nnn_expr_get(struct nnn_prog *prog, struct spar_lexinfo *info,
	     struct spar_token *token)
{
	struct nnn_expr *expr = nnn_prog_pop(prog);

	if (!spar_parse(&nnn_parser, info, token)) {
		prog->parsed = 0;
		fprintf(stderr, "Error: line %zu, %s.\n",
			info->cue.text->error_line,
			info->error.text);
	}

	info->dat.text += token->len;
	expr->line = info->cue.text->lines;

	switch (expr->type = *token->type.enum_ptr) {
	case NTWT_OP_CODE:
		nnn_set_op_code(prog, expr, token);
		break;
	case NTWT_STRING:
	        nnn_set_string(prog, expr, token);
		break;
	case NTWT_UINT:
		nnn_set_uint(prog, expr, token);
		break;
	case NTWT_INT:
		/* Handle this latter! */
	case NTWT_DOUBLE:
	        nnn_set_double(prog, expr, token);
		break;
	case NTWT_SEMI:
		expr->type = NTWT_SEMI;
		break;
	case NTWT_EOI:
		expr->type = NTWT_EOI;
		break;
	}

	return expr;
}

