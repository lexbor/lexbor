#include <lexbor/encoding/encoding.h>


int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    lxb_status_t status;
    const lxb_char_t *start, *end;
    const lxb_encoding_data_t *encoding;

    lxb_codepoint_t cp[32];
    lxb_encoding_decode_t decode = {0};
    lxb_encoding_t encoding_type = (lxb_encoding_t) (data[0] % 0x2b);

    encoding = lxb_encoding_data(encoding_type);

    status  = lxb_encoding_decode_init(&decode, encoding, cp,
                                       sizeof(cp) / sizeof(lxb_codepoint_t));
    if (status != LXB_STATUS_OK) {
        return 0;
    }

    start = (lxb_char_t *) data;
    end = start + size;

    encoding->decode(&decode, &start, end);

    return 0;
}
