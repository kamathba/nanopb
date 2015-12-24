#include <stdio.h>
#include <string.h>
#include "pb.h"
#include "pb_json.h"
#include "test.pb.h"
#include "thirdparty/jsmn.h"

#define SIZEOF_ARRAY(x) (sizeof(x)/sizeof(x[0]))

const char* jsmn_type(jsmntype_t type)
{
	switch (type) {
	case JSMN_OBJECT:
		return "OBJECT";
	case JSMN_ARRAY:
		return "ARRAY";
	case JSMN_STRING:
		return "STRING";
	case JSMN_PRIMITIVE:
		return "PRIMITIVE";
	case JSMN_UNDEFINED:
	default:
		return "UNDEFINED";
	}
}

int main(int argc, char* argv[])
{
	/* Header debug info */
	printf("nanopb-json v0.1 compiled %s %s\n", __DATE__, __TIME__);

	for (int i=0; i< argc; i++) {
		printf("%d: %s\n", i, argv[i]);
	}

//	const pb_enum_lookup_t* entry = pb_json_enum_lookup_by_name(Test2Enum_names, "Help", 4);
//
//	if(entry->name)
//		printf("%d - %s\n", entry->enum_val, entry->name);

#if 0
	jsmn_parser parser;
	jsmn_init(&parser);

	char json_string[] = "{\n\t\"foo\": true,\n\t\"bar\": 100,\n\t\"baz\": {\n\t\t\"elf\": false\n\t}\n}";
	int tokens_needed = jsmn_parse(&parser, json_string, strlen(json_string), NULL, 0);

	jsmntok_t a_tokens[tokens_needed];
	memset(a_tokens, 0, sizeof(a_tokens));
	jsmn_init(&parser);

	printf("%d %d\n", tokens_needed, sizeof(a_tokens));
	jsmntok_t* tokens = a_tokens;

	int size = 0;
	int ret;
	do {
		char *json = &json_string[parser.pos];
		ret = jsmn_parse(&parser, json, strlen(json), tokens, SIZEOF_ARRAY(a_tokens));

		printf("parser state: %d %u %u %d\n", ret, parser.pos, parser.toknext, parser.toksuper);

		if (ret == -2){
			return ret;
		}
		else if (ret <= -1) {
			size = SIZEOF_ARRAY(a_tokens);
		}
		else {
			size = ret;
		}

		for (int i=0; i < size; i++) {
			printf("%d %d %s %.*s\n", tokens[i].start, tokens[i].size, jsmn_type(tokens[i].type), \
					tokens[i].end - tokens[i].start, &json[tokens[i].start]);
		}
	} while (ret == -1);
#endif
	return 0;
}
