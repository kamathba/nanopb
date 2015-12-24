/* pb_json.h: Functions to encode and decode 
 */

#ifndef PB_JSON_H_INCLUDED
#define PB_JSON_H_INCLUDED

#include "pb.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PB_NO_JSON

struct pb_json_iformat_s {

};

struct pb_json_oformat_s {

};

struct pb_json_istream_s {

	struct pb_json_iformat_s format;

#ifndef PB_NO_ERRMSG
    const char *errmsg;
#endif
};

struct pb_json_ostream_s {
#ifdef PB_BUFFER_ONLY
    /* Callback pointer is not used in buffer-only configuration.
     * Having an int pointer here allows binary compatibility but
     * gives an error if someone tries to assign callback function.
     * Also, NULL pointer marks a 'sizing stream' that does not
     * write anything.
     */
    int *callback;
#else
    bool (*callback)(pb_json_ostream_t *stream, const char *buf, size_t count);
#endif
	struct pb_json_oformat_s format;

#ifndef PB_NO_ERRMSG
    const char *errmsg;
#endif
};

NANOPB_API const pb_enum_lookup_t* pb_json_enum_lookup_by_enum(const pb_enum_lookup_t* table, int enum_val);
NANOPB_API const pb_enum_lookup_t* pb_json_enum_lookup_by_name(const pb_enum_lookup_t* table, const char* str, int len);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef PB_JSON_H_INCLUDED */
