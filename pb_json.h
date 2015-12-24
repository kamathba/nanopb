/* pb_json.h: Functions to encode and decode 
 */

#ifndef PB_JSON_H_INCLUDED
#define PB_JSON_H_INCLUDED

#include "pb.h"

#ifdef __cplusplus
extern "C" {
#endif

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

	struct pb_json_oformat_s format;

#ifndef PB_NO_ERRMSG
    const char *errmsg;
#endif
};

struct pb_enum_lookup_s{
	int enum_val;
	const char* name;
};

NANOPB_API const pb_enum_lookup_t* pb_json_enum_lookup_by_enum(const pb_enum_lookup_t* table, int enum_val);
NANOPB_API const pb_enum_lookup_t* pb_json_enum_lookup_by_name(const pb_enum_lookup_t* table, const char* str, int len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef PB_JSON_H_INCLUDED */
