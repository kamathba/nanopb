/* pb_json.c -- encode and decode canonical JSON
 *
 * 2015 Benjamin Kamath <kamath.ben@gmail.com>
 */

#include "pb_json.h"
#include "pb_common.h"

#ifdef PB_NO_JSON
#error Cannot build pb_json with PB_NO_JSON defined
#endif

#include <inttypes.h>

/* Use the GCC warn_unused_result attribute to check that all return values
 * are propagated correctly. On other compilers and gcc before 3.4.0 just
 * ignore the annotation.
 */
#if !defined(__GNUC__) || ( __GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
    #define checkreturn
#else
    #define checkreturn __attribute__((warn_unused_result))
#endif

#ifdef PB_JSON_STRICT
#define JSMN_STRICT
#endif

//#include "jsmn.h"

/**************************************
 * Declarations internal to this file *
 **************************************/
//typedef bool (*pb_json_decoder_t)(pb_json_istream_t *stream, const pb_field_t *field, void *dest) checkreturn;
typedef bool (*pb_json_encoder_t)(pb_json_ostream_t *stream, const pb_field_t *field, const void *src) checkreturn;

static bool checkreturn buf_write(pb_json_ostream_t *stream, const uint8_t *buf, size_t count);
static bool checkreturn encode_array(pb_json_ostream_t *stream, const pb_field_t *field, const void *pData, size_t count, pb_json_encoder_t func) { return true; }
static bool checkreturn encode_field(pb_json_ostream_t *stream, const pb_field_t *field, const void *pData);
static bool checkreturn default_extension_encoder(pb_json_ostream_t *stream, const pb_extension_t *extension);
static bool checkreturn encode_extension_field(pb_json_ostream_t *stream, const pb_field_t *field, const void *pData);

static bool checkreturn pb_json_enc_varint(pb_json_ostream_t *stream, const pb_field_t *field, const void *src);
static bool checkreturn pb_json_enc_uvarint(pb_json_ostream_t *stream, const pb_field_t *field, const void *src);
static bool checkreturn pb_json_enc_svarint(pb_json_ostream_t *stream, const pb_field_t *field, const void *src);
static bool checkreturn pb_json_enc_fixed32(pb_json_ostream_t *stream, const pb_field_t *field, const void *src);
static bool checkreturn pb_json_enc_fixed64(pb_json_ostream_t *stream, const pb_field_t *field, const void *src);
static bool checkreturn pb_json_enc_bytes(pb_json_ostream_t *stream, const pb_field_t *field, const void *src);
static bool checkreturn pb_json_enc_string(pb_json_ostream_t *stream, const pb_field_t *field, const void *src);
static bool checkreturn pb_json_enc_submessage(pb_json_ostream_t *stream, const pb_field_t *field, const void *src);

/* --- Function pointers to field encoders ---
 * Order in the array must match pb_action_t LTYPE numbering.
 */
static const pb_json_encoder_t PB_JSON_ENCODERS[PB_LTYPES_COUNT] = {
    &pb_json_enc_varint,
    &pb_json_enc_uvarint,
    &pb_json_enc_svarint,
    &pb_json_enc_fixed32,
    &pb_json_enc_fixed64,

    &pb_json_enc_bytes,
    &pb_json_enc_string,
    &pb_json_enc_submessage,
    NULL /* extensions */
};

static bool checkreturn buf_write(pb_ostream_t *stream, const char *buf, size_t count)
{
	printf("%.*s", count, buf);
    return true;
}

NANOPB_API const pb_enum_lookup_t* checkreturn pb_json_enum_lookup_by_enum(const pb_enum_lookup_t* table, int enum_val)
{
	while (table->name != NULL && table->enum_val != enum_val) {
		table++;
	}

	return table;
}

NANOPB_API const pb_enum_lookup_t* checkreturn pb_json_enum_lookup_by_name(const pb_enum_lookup_t* table, const char* str, int len)
{
	while (table->name != NULL && strncmp(table->name, str, len)) {
		table++;
	}

	return table;
}

/*************************
 * Encode a single field *
 *************************/

/* Encode a static array. Handles the size calculations and possible packing. */
static bool checkreturn encode_array(pb_json_ostream_t *stream, const pb_field_t *field,
                         const void *pData, size_t count, pb_json_encoder_t func)
{
    size_t i;
    const void *p;
    size_t size;

    // [
    if (count == 0)
    	// ]
        return true;

    if (PB_ATYPE(field->type) != PB_ATYPE_POINTER && count > field->array_size)
        PB_RETURN_ERROR(stream, "array max size exceeded");

    /* We always pack arrays if the datatype allows it. */
    if (PB_LTYPE(field->type) <= PB_LTYPE_LAST_PACKABLE)
    {
        /* Write the data */
        p = pData;
        for (i = 0; i < count; i++)
        {
            if (!func(stream, field, p))
                return false;
            p = (const char*)p + field->data_size;
        }
    }
    else
    {
        p = pData;
        for (i = 0; i < count; i++)
        {
            if (!pb_encode_tag_for_field(stream, field))
                return false;

            /* Normally the data is stored directly in the array entries, but
             * for pointer-type string and bytes fields, the array entries are
             * actually pointers themselves also. So we have to dereference once
             * more to get to the actual data. */
            if (PB_ATYPE(field->type) == PB_ATYPE_POINTER &&
                (PB_LTYPE(field->type) == PB_LTYPE_STRING ||
                 PB_LTYPE(field->type) == PB_LTYPE_BYTES))
            {
                if (!func(stream, field, *(const void* const*)p))
                    return false;
            }
            else
            {
                if (!func(stream, field, p))
                    return false;
            }
            p = (const char*)p + field->data_size;
        }
    }

    return true;
}

/* Encode a field with static or pointer allocation, i.e. one whose data
 * is available to the encoder directly. */
static bool checkreturn encode_basic_field(pb_json_ostream_t *stream,
    const pb_field_t *field, const void *pData)
{
    pb_json_encoder_t func;

    union {
    	const bool* has_optional;
    	const size_t* size_repeated;
    	const size_t* which_oneof;
    } atype_arg;

    bool implicit_has = true;

    func = PB_JSON_ENCODERS[PB_LTYPE(field->type)];

    if (field->size_offset)
        atype_arg.size_repeated = (const size_t*)((const char*)pData + field->size_offset);
    else
        atype_arg.has_optional = &implicit_has;

    if (PB_ATYPE(field->type) == PB_ATYPE_POINTER)
    {
        /* pData is a pointer to the field, which contains pointer to
         * the data. If the 2nd pointer is NULL, it is interpreted as if
         * the has_field was false.
         */

        pData = *(const void* const*)pData;
        implicit_has = (pData != NULL);
    }

    switch (PB_HTYPE(field->type))
    {
        case PB_HTYPE_REQUIRED:
            if (!pData)
                PB_RETURN_ERROR(stream, "missing required field");
            if (!func(stream, field, pData))
                return false;
            break;

        case PB_HTYPE_OPTIONAL:
            if (*atype_arg.has_optional)
            {
                if (!func(stream, field, pData))
                    return false;
            }
            break;

        case PB_HTYPE_REPEATED:
            if (!encode_array(stream, field, pData, *atype_arg.size_repeated, func))
                return false;
            break;

        case PB_HTYPE_ONEOF:
            if (*atype_arg.which_oneof == field->tag)
            {
                if (!func(stream, field, pData))
                    return false;
            }
            break;

        default:
            PB_RETURN_ERROR(stream, "invalid field type");
    }

    return true;
}

static bool checkreturn pb_json_enc_varint(pb_json_ostream_t *stream, const pb_field_t *field, const void *src) { return true; }
static bool checkreturn pb_json_enc_uvarint(pb_json_ostream_t *stream, const pb_field_t *field, const void *src) { return true; }
static bool checkreturn pb_json_enc_svarint(pb_json_ostream_t *stream, const pb_field_t *field, const void *src) { return true; }
static bool checkreturn pb_json_enc_fixed32(pb_json_ostream_t *stream, const pb_field_t *field, const void *src) { return true; }
static bool checkreturn pb_json_enc_fixed64(pb_json_ostream_t *stream, const pb_field_t *field, const void *src) { return true; }
static bool checkreturn pb_json_enc_bytes(pb_json_ostream_t *stream, const pb_field_t *field, const void *src) { return true; }
static bool checkreturn pb_json_enc_string(pb_json_ostream_t *stream, const pb_field_t *field, const void *src) { return true; }
static bool checkreturn pb_json_enc_submessage(pb_json_ostream_t *stream, const pb_field_t *field, const void *src) { return true; }

