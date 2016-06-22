#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <spar/core.h>

#define NIT_SHORT_NAMES
#include <nitlib/list.h>
#include <nitlib/io.h>
#include <nitlib/socket.h>

#define NTWT_SHORT_NAMES
#include "../shared/vm/vm.h"
#include "../shared/asm/nnn_expr.h"
#include "../shared/asm/nnn_prog.h"
#include "client_io.h"

#ifdef ASSUME_UTF8
void compile_and_send(const char *charset, struct nit_connection *sock,
		      struct nnn_prog *prog, struct nnn_bcode *bcode)
{
	int error = 0;
	uint32_t msg_len;

	nnn_prog_get(prog, (uint8_t *) bcode->code, &error);
	nnn_prog_type_check(prog, &error);

	if (error)
		return;

	nnn_prog_bytecode(prog, bcode, &error);
	msg_len = bcode->size;

	if (error)
		return;

	connection_send(sock, &msg_len, sizeof(msg_len));
	connection_send(sock, bcode->code, msg_len);
}

void prompt(void)
{
	printf(u8"📮⬅️");
}

void free_conversions(void)
{

}

#else
#warning "Less efficient if your locale is utf8."
/*  C standard requires global, uninitialized variables to be
 *  implicitly initialized to zero or NULL.
 */
static uint8_t *uni_buff;
static size_t uni_size;

void compile_and_send(const char *charset, struct nit_connection *sock,
		      struct nnn_prog *prog, struct nnn_bcode *bcode)
{
	int error = 0;
	uint32_t msg_len;

	get_u8(charset, bcode->code, msg_len, &uni_buff, &uni_size);

	nnn_prog_get(prog, (uint8_t *) uni_buff, &error);
	nnn_prog_type_check(prog, &error);

	if (error)
		return;

	nnn_prog_bytecode(prog, bcode, &error);
	msg_len = bcode->size;

	if (error)
		return;

	connection_send(sock, &msg_len, sizeof(msg_len));
	connection_send(sock, bcode->code, msg_len);
}

void prompt(void)
{
	printf("* ");
}

void free_conversions(void)
{
	free(uni_buff);
}
#endif
