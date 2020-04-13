#include <lexbor/encoding/encoding.h>
#include <stdint.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
	size_t buf_length;
	lxb_status_t status;
	lxb_codepoint_t cp[32];
	lxb_encoding_decode_t decode;
	const lxb_encoding_data_t *encoding;

	lxb_encoding_t encoding_type = (lxb_encoding_t)(data[0] % 0x2b);
	encoding = lxb_encoding_data(encoding_type);
	status  = lxb_encoding_decode_init(&decode, encoding, cp, sizeof(cp) / sizeof(lxb_codepoint_t));

	if (status != LXB_STATUS_OK) {
        return 0;
	}

	const lxb_char_t *end = (lxb_char_t *)(data + size);

	status = encoding->decode(&decode, (char**)&data, end);

	return 0;
}
