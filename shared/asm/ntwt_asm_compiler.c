#include "ntwt_asm_compiler.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "../interpreter/ntwt_interpreter.h"

void command(struct ntwt_asm_tree *tree, struct ntwt_lex_info *info);

void term(struct ntwt_asm_tree *tree, struct ntwt_lex_info *info);

void lex_string(struct ntwt_lex_info *info, char *current)
{
	int backslashed = 0;
	while (1) {
		++current;
		switch (*current) {
		case '\0':
			fprintf(stderr, "Error: string has no "
				"ending quote on line %u\n", info->lineno);
			exit(1);
		case '\\':
			backslashed = !backslashed;
			break;
		case '\n':
			++info->lineno; backslashed = 0;
			break;
		case '\"':
			if (!backslashed) {
				info->lexlen = current - info->lexme;
				info->offset = 1;
			        info->token =  NTWT_STRING;
				return;
			}
		default:
			backslashed = 0;
			break;
		}
	}
}

void lex_num(struct ntwt_lex_info *info, char *current)
{
	unsigned int num_type = NTWT_UINT;
	int period = 0;
	while (1) {
		++current;
		switch (*current) {
		case '0' ... '9' : continue;
		case '\n' : ++info->lineno;
		case ' '  :
		case '\t' :
		case '\v' :
		case '\f' :
		case '\r' :
		case ';'  :
			info->lexlen = current - info->lexme;
			info->token = num_type;
			return;
		case '.' :
			if (period) {
				fprintf(stderr, "Error: to many '.' in "
					"number on line %u\n", info->lineno);
				exit(1);
			}
			num_type = NTWT_DOUBLE;
			break;
		case '\0' :
			fprintf(stderr, "Error: unexpected end of input "
				"on line %u\n", info->lineno);
			exit(1);
		default :
			fprintf(stderr, "Error: invalid char '%c' in "
				"number on line %u\n", *current,
				info->lineno);
			exit(1);
		}
	}
}

void lex_op_code(struct ntwt_lex_info *info, char *current)
{
	while(!isspace(*current) && *current != ';') {
		++current;
	}

	info->lexlen = current - info->lexme;
        info->token = NTWT_OP_CODE;
}

void lex(struct ntwt_lex_info *info)
{
	char *current = info->lexme;
	current += info->lexlen + info->offset;

	while (isspace(*current)) {
		if (*current == '\n')
			++info->lineno;
		++current;
	}

	info->lexlen = 1;
	info->offset = 0;
	info->lexme = current;

	switch (*current) {
	case '\0':
		info->token = NTWT_EOI;
		break;
	case ';' :
		info->token = NTWT_SEMICOLON;
	        break;
	case '"' :
		++info->lexme; lex_string(info, current);
	        break;
	case '0' ... '9':
		lex_num(info, current);
		break;
	case '*' :
		/* Assumes an op code after '*' */
		++info->lexme;
		lex_op_code(info, current);
		break;
	default  :
		/* Assumes an  op code after ';' */
		if (info->token != NTWT_SEMICOLON) {
			fprintf(stderr,
				"Error: invalid argument on line %u, if you "
				"want an op_code, put '*' before it.\n",
				info->lineno);
			exit(1);
		}
		lex_op_code(info, current);
	        break;
	}
}



static int match(struct ntwt_lex_info *info, unsigned int token)
{
	return info->token == token;
}

static void advance(struct ntwt_lex_info *info)
{
	lex(info);
}

struct ntwt_asm_tree *statements(char *code)
{
	struct ntwt_lex_info info = {
		.lexme = code,
		.lexlen = 0,
		.lineno = 0,
		.token = NTWT_SEMICOLON
	};
	struct ntwt_asm_tree *root = malloc(sizeof(struct ntwt_asm_tree));

	struct ntwt_asm_tree *tree = root;

	advance(&info);
	command(tree, &info);
	if (!match(&info, NTWT_SEMICOLON)) {
		fprintf(stderr, "Error: Inserting missing semicolon "
			"on line %u\n", info.lineno);
		exit(1);
	} else {
		advance(&info);
	}

	/* printf("token: %u\n", info.token); */
        while (!match(&info, NTWT_EOI)) {
		tree->next = malloc(sizeof(struct ntwt_asm_tree));
		tree = tree->next;
		command(tree, &info);
		if (match(&info, NTWT_SEMICOLON)) {
			advance(&info);
		} else {
			fprintf(stderr, "Error: Inserting missing semicolon "
				"on line %u\n", info.lineno);
			exit(1);
		}
	}

	return root;
	/* printf("token: %u, %u, %.*s\n", a.token, a.lexlen, a.lexlen, a.lexme); */
}

void command(struct ntwt_asm_tree *tree, struct ntwt_lex_info *info)
{
	struct ntwt_asm_tree *branch;

	tree->type = NTWT_COMMAND;
	tree->next = NULL;
	tree->contents.branch = malloc(sizeof(struct ntwt_asm_tree));

	branch = tree->contents.branch;
	term(branch, info);
	advance(info);
	while (!match(info, NTWT_SEMICOLON)) {
		if (match(info, NTWT_EOI)) {
			printf("Error: end of input on line: %u\n",
				info->lineno);
			exit(1);
		}
	        branch->next = malloc(sizeof(struct ntwt_asm_tree));
		branch = branch->next;
		branch->next = NULL;
		term(branch, info);
		advance(info);
	}
}

void term(struct ntwt_asm_tree *tree, struct ntwt_lex_info *info)
{
	switch (tree->type = info->token) {
	case NTWT_UINT:
		tree->contents.integer = strtoul(info->lexme, NULL, 0);
		break;
	case NTWT_DOUBLE:
		tree->contents.decimal = strtod(info->lexme, NULL);
		break;
	case NTWT_STRING:
		tree->contents.string = malloc(info->lexlen + 1);
		strncpy(tree->contents.string, info->lexme, info->lexlen);
		break;
	case NTWT_OP_CODE:
		if (!strncmp(info->lexme, "TEST", info->lexlen))
			tree->contents.op_code = NTWT_OP_TEST;
		else {
			fprintf(stderr, "Error: unrecognized op code\n");
			exit(1);
		}
		break;
	default:
		fprintf(stderr, "Error: unrecognized token\n");
		exit(1);
	}
}

/* void ntwt_write_tree(struct ntwt_write_tree *tree, FILE *image) */
/* { */
/* 	switch(tree->type) { */
/* 	case NTWT_OP_CODE: */
/* 		fwrite(&tree->contents.op_code, sizeof(char), 1, image); */
/* 		break; */
/* 	case NTWT_UINT: */
/* 		fwrite(&tree->contents.integer, sizeof(unsigned int), 1, image); */
/* 		break; */
/* 	case NTWT_DOUBLE: */
/* 		fwrite(tree->contents.string, sizeof(char), */
/* 		       strlen(tree->contents.string) + 1, image); */
/* 		break; */


/* 	} */
/* } */

/* void ntwt_bytecode_write(struct ntwt_asm_tree *tree) */
/* { */
/* 	remove("state.ilk"); */
/* 	FILE *image = fopen("state.ilk", "ab"); */
/*         fwrite(code, sizeof(char), strlen(code), image); */
/* 	fclose(image); */
/* } */
