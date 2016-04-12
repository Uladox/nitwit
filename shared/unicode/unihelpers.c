#include <uniconv.h>
#include <unistr.h>
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>

#include "unihelpers.h"

void get_u8(const char *charset, const char *src, size_t src_size,
	    uint8_t **result, size_t *result_size)
{
	uint8_t *tmp_u8;
	size_t tmp_size = *result_size;

	tmp_u8 = u8_conv_from_encoding(charset, iconveh_question_mark, src,
				       src_size, NULL, *result, &tmp_size);
	if (tmp_u8 != *result && *result)
		free(*result);
	if (tmp_size > *result_size)
		*result_size = tmp_size;
	*result = tmp_u8;
}

void get_encoded(const char *charset, const uint8_t *src, size_t src_size,
		 char **result, size_t *result_size)
{
	char *tmp_encoded;
	size_t tmp_size = *result_size;

	tmp_encoded = u8_conv_to_encoding(charset, iconveh_question_mark, src,
					  src_size, NULL, *result, &tmp_size);
	if (tmp_encoded != *result && *result)
		free(*result);
	if (tmp_size > *result_size)
		*result_size = tmp_size;
	*result = tmp_encoded;
}

