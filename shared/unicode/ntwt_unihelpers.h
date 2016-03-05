#ifndef NTWT_UNIHELPERS_H
#define NTWT_UNIHELPERS_H

#include <stdint.h>
#include <stdlib.h>

void get_u8(const char *charset, const char *src, size_t src_size,
	    uint8_t **result, size_t *result_size);

void get_encoded(const char *charset, const uint8_t *src, size_t src_size,
		 char **result, size_t *result_size);

#endif
