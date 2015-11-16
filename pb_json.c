/* pb_json.c -- encode to and decode from protobuf compliant JSON
 *
 * 2015 Benjamin Kamath <kamath.ben@gmail.com>
 */

#include "pb.h"
#include "pb_json.h"
#include "pb_common.h"

/* Use the GCC warn_unused_result attribute to check that all return values
 * are propagated correctly. On other compilers and gcc before 3.4.0 just
 * ignore the annotation.
 */
#if !defined(__GNUC__) || ( __GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
    #define checkreturn
#else
    #define checkreturn __attribute__((warn_unused_result))
#endif

static const pb_json_encoder_t PB_JSON_ENCODE_DEFAULTS = {

};

const char *message_formatter = "{\n%s\n}";
const char *array_formatter = "[%s]";

static const pb_json_decoder_t PB_JSON_DECODE_DEFAULTS = {

};

bool checkreturn pb_json_encode(pb_ostream_t *stream,
#ifndef PB_JSON_BASIC_FORMATTING
                    pb_json_encoder_t *options,
#endif
                    const pb_field_t fields[],
                    const void *src_struct)
{
    pb_json_encoder_t *format = &PB_JSON_ENCODE_DEFAULTS;

#ifndef PB_JSON_BASIC_FORMATTING
    if (options)
    {
        format = options;
    }
#endif

    pb_field_iter_t iter;
    if (!pb_field_iter_begin(&iter, fields, remove_const(src_struct)))
        return true; /* Empty message type */
    
    do {
        if (PB_LTYPE(iter.pos->type) == PB_LTYPE_EXTENSION)
        {
            /* Special case for the extension field placeholder */
            if (!encode_extension_field(stream, iter.pos, iter.pData))
                return false;
        }
        else
        {
            /* Regular field */
            if (!encode_field(stream, iter.pos, iter.pData))
                return false;
        }
    } while (pb_field_iter_next(&iter));
    
    return true;
}
