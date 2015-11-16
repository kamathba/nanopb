/*
 */

#ifndef PB_JSON_H_INCLUDED
#define PB_JSON_H_INCLUDED

#include "pb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pb_json_encoder_s
{
    const char* whitespace_after_bracket;
    const char* option_2;
} pb_json_encoder_t;

typedef struct pb_json_decoder_s
{
    bool strict;
} pb_json_decoder_t;

/***************************
 * Main encoding functions *
 ***************************/

bool pb_json_encode(            pb_ostream_t *stream,
#ifndef PB_JSON_BASIC_FORMATTING
                                pb_json_encoder_t *format,
#endif
                                const pb_field_t fields[],
                                const void *src_struct);

bool pb_json_decode(            pb_istream_t *stream,
#ifndef PB_JSON_STRICT_PARSER
                                pb_json_decoder_t *options,
#endif
                                const pb_field_t fields[],
                                const void *src_struct);

bool pb_json_get_encoded_size(  size_t *size,
#ifndef PB_JSON_BASIC_FORMATTING
                                pb_json_encoder_t *format,
#endif
                                const pb_field_t fields[],
                                const void *src_struct);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
