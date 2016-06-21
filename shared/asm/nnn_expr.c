#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include <nitlib/list.h>
#include <nitlib/hashmap.h>

#include <spar/core.h>
#include <spar/text_utils.h>
#include <spar/strlit_parser.h>
#include <spar/word_parser.h>
#include <spar/num_parser.h>

/* #include "../vm/state.h" */
/* #include "../vm/vm.h" */
#include "../../gen/output/op_map.h"
#include "nnn_parser.h"
#include "nnn_expr.h"

#include <spar/token_print.h>

static inline struct  nnn_expr*
pop(struct nnn_prog *prog)
{
	if (prog->stack) {
		struct nnn_expr *tmp = prog->stack;

		prog->stack = LIST_NEXT(tmp);
		return tmp;
	}

	return malloc(sizeof(*prog->stack));
}

static inline void
nnn_set_op_code(struct nnn_prog *prog, struct nnn_expr *expr,
		struct spar_token *token, enum spar_parsed *error)
{
	expr->type = NTWT_OP_CODE;
	prog->size += (expr->size = sizeof(expr->dat.op_code));

	if (*error)
		return;

	char *result = hashmap_get(&ntwt_op_map, token->dat.text, token->len);

	if (!result) {
		*error = SPAR_ERROR;
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
	       struct spar_token *token, enum spar_parsed *error)
{
	char *str = malloc(token->data_size);

	expr->type = NTWT_STRING;
	prog->size += (expr->size = token->data_size);

	strncpy(str, token->dat.text + 1, token->len - 1);
	str[token->data_size - 1] = '\0';

	expr->dat.string = str;
}

static inline void
nnn_set_uint(struct nnn_prog *prog, struct nnn_expr *expr,
	     struct spar_token *token)
{
	expr->type = NTWT_UINT;
	prog->size += (expr->size = sizeof(expr->dat.integer));
	expr->dat.integer = strtoul(token->dat.text, NULL, 0);
}

static inline void
nnn_set_double(struct nnn_prog *prog, struct nnn_expr *expr,
	       struct spar_token *token)
{
	expr->type = NTWT_DOUBLE;
	prog->size += (expr->size = sizeof(expr->dat.decimal));
	expr->dat.integer = strtod(token->dat.text, NULL);
}


struct nnn_expr *
nnn_expr_get(struct nnn_prog *prog, struct spar_lexinfo *info,
	     struct spar_token *token, enum spar_parsed *error)
{
	struct nnn_expr *expr = pop(prog);

	if (spar_parse(&nnn_parser, info, token) == SPAR_ERROR) {
		*error = SPAR_ERROR;
		fprintf(stderr, "Error: line %zu, %s.\n",
			info->cue.text->error_line,
			info->error.text);
	}

	info->dat.text += token->len;
	expr->line = info->cue.text->lines;

	if (token->type == spar_type_word) {
		nnn_set_op_code(prog, expr, token, error);
	} else if (token->type == spar_type_strlit) {
	        nnn_set_string(prog, expr, token, error);
	} else if (token->type == spar_type_whole) {
		nnn_set_uint(prog, expr, token);
	} else if (token->type == spar_type_decimal) {
	        nnn_set_double(prog, expr, token);
	} else if (token->type == nnn_type_semi) {
		expr->type = NTWT_SEMI;
	} else if (token->type == spar_type_end) {
		expr->type = NTWT_EOI;
	}

	return expr;
}

