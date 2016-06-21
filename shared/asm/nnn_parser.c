#include <stdlib.h>

#include <spar/core.h>
#include <spar/cases.h>
#include <spar/meta.h>
#include <spar/text_utils.h>
#include <spar/strlit_parser.h>
#include <spar/word_parser.h>
#include <spar/num_parser.h>

char nnn_type_semi[] = ";";

static enum spar_parsed
nnn_parse(struct spar_parser *parser, struct spar_lexinfo *info,
	  struct spar_token *token);

SPAR_PARSER_INIT(nnn_basic, "nnn_basic", nnn_parse, NULL);
SPAR_MOD_INIT_META(nnn_parser, &nnn_basic);

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
	token->type = nnn_type_semi;
	token->len = 1;
	token->dat.text = info->dat.text;
}

static inline void
token_set_end(struct spar_token *token, struct spar_lexinfo *info)
{
	token->type = spar_type_end;
	token->len = SPAR_UNKNOWN_SIZE;
	token->dat.text = info->dat.text;
}

static enum spar_parsed
nnn_parse(struct spar_parser *parser, struct spar_lexinfo *info,
	  struct spar_token *token)
{
	spar_skip_blank(info);

	if (*info->dat.text == '\0') {
		token_set_end(token, info);
		return SPAR_END;
	} else if (token->type == nnn_type_semi) {
		spar_token_set_parser(token, &spar_word_parser);
		return SPAR_OK;
	}

	switch (*info->dat.text) {
	SPAR_DIGIT_CASES:
		spar_token_set_parser(token, &spar_num_parser);
		return SPAR_OK;
	case '\"':
		spar_token_set_parser(token, &spar_strlit_parser);
		return SPAR_OK;
	case '*':
		spar_token_set_parser(token, &spar_word_parser);
		return SPAR_OK;
	case ';':
		token_set_semi(token, info);
		return SPAR_OK;
	default:
		spar_parse(&spar_word_parser, info, token);
		invalid_op_code(info);
		return SPAR_ERROR;
	}
}
