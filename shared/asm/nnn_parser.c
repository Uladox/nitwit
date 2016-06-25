#include <stdlib.h>

#include <spar/core.h>
#include <spar/cases.h>
#include <spar/text_utils.h>
#include <stdint.h>

#include <spar/meta.h>
#include <spar/cusstrlit.h>
#include <spar/cusword.h>
#include <spar/cusnum.h>

#include "../vm/vm.h"

/* Unchanging global variables holding token type values for use in switch. */
const enum ntwt_token nnn_eoi     = NTWT_EOI;
const enum ntwt_token nnn_semi    = NTWT_SEMI;
const enum ntwt_token nnn_op_code = NTWT_OP_CODE;
const enum ntwt_token nnn_uint    = NTWT_UINT;
const enum ntwt_token nnn_double  = NTWT_DOUBLE;
const enum ntwt_token nnn_string  = NTWT_STRING;

/* Throw-away function to keep nnn_parse simple. */
static inline void
invalid_op_code(struct spar_lexinfo *info)
{
	spar_text_error(info,
			"invalid argument, use * for op code, "
			"if that is what you want", 0);
}

static inline void
token_set_semi(struct spar_token *token, struct spar_lexinfo *info)
{
	token->type.generic = &nnn_semi;
	token->len = 1;
	token->dat.text = info->dat.text;
}

static inline void
token_set_end(struct spar_token *token, struct spar_lexinfo *info)
{
	token->type.generic = &nnn_eoi;
	token->len = SPAR_UNKNOWN_SIZE;
	token->dat.text = info->dat.text;
}

/* What you came here for. */
static enum spar_parsed
nnn_parse(struct spar_parser *parser, struct spar_lexinfo *info,
	  struct spar_token *token)
{
	spar_skip_blank(info);

	if (*info->dat.text == '\0') {
		token_set_end(token, info);
		return SPAR_END;
	} else if (token->type.generic == &nnn_semi) {
		spar_token_set_parser(token, &op_code_parser);
		return SPAR_OK;
	}

	switch (*info->dat.text) {
	SPAR_DIGIT_CASES:
		spar_token_set_parser(token, &num_parser);
		return SPAR_OK;
	case '\"':
		spar_token_set_parser(token, &strlit_parser);
		return SPAR_OK;
	case '*':
		spar_token_set_parser(token, &op_code_parser);
		return SPAR_OK;
	case ';':
		token_set_semi(token, info);
		return SPAR_OK;
	default:
		spar_parse(&op_code_parser, info, token);
		invalid_op_code(info);
		return SPAR_ERROR;
	}
}

/* Setting up sub-parsers using spar macros. */
static SPAR_PARSE_FUNC(parse_op_code)
{
        SPAR_BODY_CUSWORD(&nnn_op_code, sizeof(char), SPAR_SEP_CASES, 0);
}

static SPAR_PARSE_FUNC(parse_strlit)
{
	SPAR_BODY_CUSSTRLIT(&nnn_string, 0);
}

static SPAR_PARSE_FUNC(parse_num)
{
	SPAR_BODY_CUSNUM(&nnn_uint, sizeof(uint32_t),
			 &nnn_double, sizeof(double),
			 SPAR_SEP_CASES, 0);
}

static SPAR_PARSER_INIT(op_code_parser, "parse_op_code", parse_op_code, NULL);
static SPAR_PARSER_INIT(strlit_parser,  "parse_strlit",  parse_strlit,  NULL);
static SPAR_PARSER_INIT(num_parser,     "parse_num",     parse_num,     NULL);
static SPAR_PARSER_INIT(nnn_basic,      "nnn_basic",     nnn_parse,     NULL);

/* The full parser struct, ready for export! */
SPAR_MOD_INIT_META(nnn_parser, &nnn_basic);
