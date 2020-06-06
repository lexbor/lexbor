#include <lexbor/encoding/encoding.h>


#ifndef LEXBOR_HAVE_FUZZER
int
main(int argc, const char *argv[])
{
    size_t size;
    lxb_status_t status;
    const lxb_encoding_data_t *encoding;

    bool loop = true;

    /* Decode */
    char inbuf[4096];
    const lxb_char_t *data, *end;

    lxb_codepoint_t cp[4096];

    lxb_encoding_decode_t decode = {0};
    lxb_encoding_t encoding_type = (lxb_encoding_t) (argc + 1);

    encoding = lxb_encoding_data(encoding_type);

    status = lxb_encoding_decode_init(&decode, encoding, cp,
                                      sizeof(cp) / sizeof(lxb_codepoint_t));
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    do {
        size = fread(inbuf, 1, sizeof(inbuf), stdin);
        if (size != sizeof(inbuf)) {
            if (feof(stdin)) {
                loop = false;
            }
            else {
                return EXIT_FAILURE;
            }
        }

        data = (const lxb_char_t *) inbuf;
        end = data + size;

        encoding->decode(&decode, &data, end);
    }
    while (loop);

    return EXIT_SUCCESS;
}
#endif

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
