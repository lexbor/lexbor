/*
 * Copyright (C) 2023-2024 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <inttypes.h>

#include <lexbor/url/url.h>
#include <lexbor/core/fs.h>
#include <unit/test.h>
#include <unit/kv.h>


typedef struct {
    lexbor_str_t href;
    lexbor_str_t protocol;
    lexbor_str_t username;
    lexbor_str_t password;
    lexbor_str_t host;
    lexbor_str_t hostname;
    lexbor_str_t port;
    lexbor_str_t pathname;
    lexbor_str_t search;
    lexbor_str_t hash;
}
url_change_t;

typedef struct {
    lexbor_str_t   url;
    lexbor_str_t   base;
    lexbor_str_t   done;
    lexbor_str_t   scheme;
    lexbor_str_t   username;
    lexbor_str_t   password;
    lexbor_str_t   host;
    int64_t        port;
    bool           has_port;
    lexbor_str_t   path;
    size_t         path_length;
    bool           has_path_length;
    lexbor_str_t   query;
    lexbor_str_t   fragment;
    bool           failed;
    lxb_encoding_t encoding;
    url_change_t   change;
    bool           has_change;
}
url_entry_t;

typedef struct {
    unit_kv_t        *kv;
    lexbor_mraw_t    *mraw;
    lxb_url_parser_t *parser;
    lexbor_str_t     str;
    lxb_status_t     status;
    size_t           err_count;
    size_t           total;
}
url_context_t;


static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx);

static lxb_status_t
test_changes_set(lxb_url_t *url, url_entry_t *entry);

static lxb_status_t
test_value_to_entry(unit_kv_value_t *value, url_entry_t *entry);

static lxb_status_t
test_value_changes_to_entry(unit_kv_value_t *value, url_entry_t *entry);

static lxb_status_t
test_value_to_str(unit_kv_value_t *value, lexbor_str_t *str, const char *key,
                  bool is_req);

static lxb_status_t
test_value_to_number(unit_kv_value_t *value, int64_t *num, const char *key,
                     bool is_req, bool *is_exist);

static lxb_status_t
test_value_to_bool(unit_kv_value_t *value, bool *isas, const char *key,
                   bool is_req);

static lxb_status_t
test_value_encoding(unit_kv_value_t *value, lxb_encoding_t *enc);

static lxb_status_t
test_check_str(lexbor_str_t *have, const lexbor_str_t *need, const char *name);


lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    lxb_char_t *p;
    url_context_t *context = ctx;

    p = lexbor_str_append(&context->str, context->mraw, data, len);

    return (p != NULL) ? LXB_STATUS_OK : LXB_STATUS_ERROR;
}

int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    url_context_t ctx;
    const char *dir_path;

    if (argc < 2) {
        printf("Usage:\n\tparser <directory path>\n");
        return EXIT_FAILURE;
    }

    dir_path = argv[1];

    ctx.parser = lxb_url_parser_create();
    status = lxb_url_parser_init(ctx.parser, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    ctx.mraw = ctx.parser->mraw;
    ctx.status = LXB_STATUS_OK;
    ctx.err_count = 0;
    ctx.total = 0;
    ctx.str.data = NULL;
    ctx.str.length = 0;

    ctx.kv = unit_kv_create();
    status = unit_kv_init(ctx.kv, 256);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    status = lexbor_fs_dir_read((const lxb_char_t *) dir_path,
                                LEXBOR_FS_DIR_OPT_WITHOUT_DIR
                                |LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN,
                                file_callback, &ctx);
    if (status != LXB_STATUS_OK) {
        ctx.err_count = 1;

        TEST_PRINTLN("Failed to read directory: %s", dir_path);
    }

    lxb_url_parser_memory_destroy(ctx.parser);
    lxb_url_parser_destroy(ctx.parser, true);

    unit_kv_destroy(ctx.kv, true);

    printf("Total: " LEXBOR_FORMAT_Z "\n", ctx.total);
    printf("Errors: " LEXBOR_FORMAT_Z "\n", ctx.err_count);

    return (ctx.err_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx)
{
    bool have_error;
    lxb_url_t *url, *base;
    lxb_char_t *p;
    url_entry_t entry;
    lexbor_str_t str;
    lxb_status_t status;
    url_context_t *context;
    unit_kv_value_t *value;
    unit_kv_array_t *entries;

    if (filename_len < 5 ||
        strncmp((const char *) &filename[ (filename_len - 4) ], ".ton", 4) != 0)
    {
        return LEXBOR_ACTION_OK;
    }

    context = ctx;

    status = unit_kv_parse_file(context->kv, (const lxb_char_t *) fullpath);
    if (status != LXB_STATUS_OK) {
        str = unit_kv_parse_error_as_string(context->kv);

        TEST_PRINTLN("%s", str.data);

        unit_kv_string_destroy(context->kv, &str, false);

        context->status = LXB_STATUS_ERROR;
        return LEXBOR_ACTION_STOP;
    }

    value = unit_kv_value(context->kv);
    if (value == NULL) {
        TEST_PRINTLN("Failed to get root value");
        return EXIT_FAILURE;
    }

    if (unit_kv_is_array(value) == false) {
        TEST_PRINTLN("Root value is not array");

        context->status = LXB_STATUS_ERROR;
        return LEXBOR_ACTION_STOP;
    }

    entries = unit_kv_array(value);

    for (size_t i = 0; i < entries->length; i++) {
        status = test_value_to_entry(entries->list[i], &entry);
        if (status != LXB_STATUS_OK) {
            printf("Failed to serialize url.\n");
            return EXIT_FAILURE;
        }

        base = NULL;

        if (entry.base.data != NULL) {
            base = lxb_url_parse(context->parser, NULL, entry.base.data,
                                 entry.base.length);
            if (base == NULL) {
                printf("Failed to parse base URL: %.*s\n",
                       (int) entry.base.length,
                       (const char *) entry.base.data);

                return EXIT_FAILURE;
            }

            lxb_url_parser_clean(context->parser);
        }

        status = lxb_url_parse_basic(context->parser, NULL, base, entry.url.data,
                                     entry.url.length, LXB_URL_STATE__UNDEF,
                                     entry.encoding);
        if (status != LXB_STATUS_OK) {
            if (!entry.failed) {
                context->err_count += 1;

                printf("Failed to parse: %.*s\n", (int) entry.url.length,
                       (const char *) entry.url.data);

                return EXIT_FAILURE;
            }
        }

        url = context->parser->url;

        have_error = false;

        /* URL */

        printf("URL: %.*s\n", (int) entry.url.length,
               (const char *) entry.url.data);

        if (entry.base.data != NULL) {
            printf("Base URL: %.*s\n", (int) entry.base.length,
                   (const char *) entry.base.data);
        }

        if (status != LXB_STATUS_OK) {
            printf("Failed to parse: ok, expected.\n\n");

            context->total += 1;
            goto done;
        }

        printf("Expecting: %.*s\n", (int) entry.done.length,
               (const char *) entry.done.data);

        status = test_changes_set(url, &entry);
        if (status != LXB_STATUS_OK) {
            if (!entry.failed) {
                context->err_count += 1;

                printf("Failed to change URL object: %.*s\n",
                       (int) entry.url.length, (const char *) entry.url.data);

                return EXIT_FAILURE;
            }
        }

        p = lexbor_str_init(&context->str, context->mraw, 1024);
        if (p == NULL) {
            return EXIT_FAILURE;
        }

        context->str.length = 0;

        status = lxb_url_serialize(url, callback, context, false);
        if (status != LXB_STATUS_OK) {
            printf("Failed to serialize url.\n");
            return EXIT_FAILURE;
        }

        status = test_check_str(&context->str, &entry.done, "Full comparison");
        if (status != LXB_STATUS_OK) {
            have_error = true;
        }

        /* Username */

        status = test_check_str(&url->username, &entry.username, "Username");
        if (status != LXB_STATUS_OK) {
            have_error = true;
        }

        /* Password */

        status = test_check_str(&url->password, &entry.password, "Password");
        if (status != LXB_STATUS_OK) {
            have_error = true;
        }

        /* Host */

        context->str.length = 0;

        status = lxb_url_serialize_host(&url->host, callback, context);
        if (status != LXB_STATUS_OK) {
            printf("Failed to serialize host.\n");
            return EXIT_FAILURE;
        }

        status = test_check_str(&context->str, &entry.host, "Host");
        if (status != LXB_STATUS_OK) {
            have_error = true;
        }

        /* Port */

        if (url->has_port != entry.has_port) {
            printf("Port: not equal\n");

            if (url->has_port) {
                printf("    Have: %d\n", url->port);
            }
            else {
                printf("    Have: NULL\n");
            }

            if (entry.has_port) {
                printf("    Need: %" PRId64 "\n", entry.port);
            }
            else {
                printf("    Need: NULL\n");
            }

            have_error = true;
        }
        else if (url->has_port) {
            if (url->port != entry.port) {
                printf("Port: not equal\n");
                printf("    Have: %d\n", url->port);
                printf("    Need: %" PRId64 "\n", entry.port);

                have_error = true;
            }
            else {
                printf("Port: ok\n");
            }
        }

        /* Path */

        context->str.length = 0;

        status = lxb_url_serialize_path(&url->path, callback, context);
        if (status != LXB_STATUS_OK) {
            printf("Failed to serialize path.\n");
            return EXIT_FAILURE;
        }

        status = test_check_str(&context->str, &entry.path, "Path");
        if (status != LXB_STATUS_OK) {
            have_error = true;
        }

        /* Path length */

        if (entry.has_path_length) {
            if (url->path.length != entry.path_length) {
                printf("Path Length: not equal\n");
                printf("    Have: " LEXBOR_FORMAT_Z "\n", url->path.length);
                printf("    Need: " LEXBOR_FORMAT_Z "\n", entry.path_length);

                have_error = true;
            }
            else {
                printf("Path Length: ok\n");
            }
        }

        /* Query */

        status = test_check_str(&url->query, &entry.query, "Query");
        if (status != LXB_STATUS_OK) {
            have_error = true;
        }

        /* Fragment */

        status = test_check_str(&url->fragment, &entry.fragment, "Fragment");
        if (status != LXB_STATUS_OK) {
            have_error = true;
        }

        printf("\n");

        if (have_error) {
            context->err_count += 1;
        }

        context->total += 1;

    done:

        lexbor_mraw_clean(context->parser->mraw);
        lxb_url_parser_clean(context->parser);
    }

    unit_kv_clean(context->kv);

    return LEXBOR_ACTION_OK;
}

static lxb_status_t
test_changes_set(lxb_url_t *url, url_entry_t *entry)
{
    lxb_status_t status;
    url_change_t *change;

    if (!entry->has_change) {
        return LXB_STATUS_OK;
    }

    change = &entry->change;

    if (change->href.data != NULL) {
        status = lxb_url_api_href_set(url, NULL,
                                      change->href.data, change->href.length);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (change->protocol.data != NULL) {
        status = lxb_url_api_protocol_set(url, NULL, change->protocol.data,
                                          change->protocol.length);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (change->username.data != NULL) {
        status = lxb_url_api_username_set(url, change->username.data,
                                          change->username.length);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (change->password.data != NULL) {
        status = lxb_url_api_password_set(url, change->password.data,
                                          change->password.length);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (change->host.data != NULL) {
        status = lxb_url_api_host_set(url, NULL, change->host.data,
                                      change->host.length);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (change->hostname.data != NULL) {
        status = lxb_url_api_hostname_set(url, NULL, change->hostname.data,
                                          change->hostname.length);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (change->port.data != NULL) {
        status = lxb_url_api_port_set(url, NULL,
                                      change->port.data, change->port.length);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (change->pathname.data != NULL) {
        status = lxb_url_api_pathname_set(url, NULL, change->pathname.data,
                                          change->pathname.length);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (change->search.data != NULL) {
        status = lxb_url_api_search_set(url, NULL, change->search.data,
                                        change->search.length);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (change->hash.data != NULL) {
        status = lxb_url_api_hash_set(url, NULL, change->hash.data,
                                      change->hash.length);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}


static lxb_status_t
test_value_to_entry(unit_kv_value_t *value, url_entry_t *entry)
{
    int64_t num;
    lxb_status_t status;

    if (unit_kv_is_hash(value) == false) {
        TEST_PRINTLN("Entry is not hash");
        return LXB_STATUS_ERROR;
    }

    status = test_value_to_str(value, &entry->url, "url", true);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(value, &entry->base, "base", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(value, &entry->done, "done", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(value, &entry->scheme, "scheme", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(value, &entry->username, "username", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(value, &entry->password, "password", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(value, &entry->host, "host", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_number(value, &entry->port, "port", false, NULL);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_bool(value, &entry->has_port, "has_port", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(value, &entry->path, "path", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_number(value, &num,
                                  "path_length", false, &entry->has_path_length);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    entry->path_length = 0;

    if (num > 0) {
        entry->path_length = (size_t) num;
    }

    status = test_value_to_str(value, &entry->query, "query", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(value, &entry->fragment, "fragment", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_bool(value, &entry->failed, "failed", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_encoding(value, &entry->encoding);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return test_value_changes_to_entry(value, entry);
}

static lxb_status_t
test_value_changes_to_entry(unit_kv_value_t *value, url_entry_t *entry)
{
    lxb_status_t status;
    url_change_t *change;
    unit_kv_value_t *change_value;

    memset(&entry->change, 0x00, sizeof(url_change_t));

    entry->has_change = false;

    change_value = unit_kv_hash_value_nolen_c(value, "change");
    if (change_value == NULL) {
        return LXB_STATUS_OK;
    }

    if (!unit_kv_is_hash(change_value)) {
        if (unit_kv_is_null(change_value)) {
            return LXB_STATUS_OK;
        }

        TEST_PRINTLN("Parameter 'change' must be a HASH");
        return LXB_STATUS_ERROR;
    }

    change = &entry->change;
    entry->has_change = true;

    status = test_value_to_str(change_value, &change->href, "href", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(change_value, &change->protocol,
                               "protocol", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(change_value, &change->username,
                               "username", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(change_value, &change->password,
                               "password", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(change_value, &change->host, "host", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(change_value, &change->hostname,
                               "hostname", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(change_value, &change->port, "port", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(change_value, &change->pathname,
                               "pathname", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = test_value_to_str(change_value, &change->search, "search", false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return test_value_to_str(change_value, &change->hash, "hash", false);
}

static lxb_status_t
test_value_to_str(unit_kv_value_t *value, lexbor_str_t *str, const char *key,
                  bool is_req)
{
    unit_kv_value_t *src;

    str->data = NULL;
    str->length = 0;

    src = unit_kv_hash_value_nolen_c(value, key);
    if (src == NULL) {
        if (is_req) {
            TEST_PRINTLN("Required parameter missing: %s", key);
            return LXB_STATUS_ERROR;
        }

        return LXB_STATUS_OK;
    }

    if (!unit_kv_is_string(src)) {
        if (unit_kv_is_null(src)) {
            return LXB_STATUS_OK;
        }

        TEST_PRINTLN("Parameter '%s' must be a STRING", key);
        return LXB_STATUS_ERROR;
    }

    *str = *unit_kv_string(src);

    return LXB_STATUS_OK;
}

static lxb_status_t
test_value_to_number(unit_kv_value_t *value, int64_t *num, const char *key,
                     bool is_req, bool *is_exist)
{
    unit_kv_value_t *src;
    unit_kv_number_t *number;

    *num = 0;

    if (is_exist) {
        *is_exist = false;
    }

    src = unit_kv_hash_value_nolen_c(value, key);
    if (src == NULL) {
        if (is_req) {
            TEST_PRINTLN("Required parameter missing: %s", key);
            return LXB_STATUS_ERROR;
        }

        return LXB_STATUS_OK;
    }

    if (!unit_kv_is_number(src)) {
        TEST_PRINTLN("Parameter '%s' must be a NUMBER", key);
        return LXB_STATUS_ERROR;
    }

    number = unit_kv_number(src);

    if (number->is_float) {
        TEST_PRINTLN("Parameter '%s' should not be float", key);
        return LXB_STATUS_ERROR;
    }

    *num = number->value.i;

    if (is_exist) {
        *is_exist = true;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
test_value_to_bool(unit_kv_value_t *value, bool *isas, const char *key,
                   bool is_req)
{
    unit_kv_value_t *src;

    *isas = false;

    src = unit_kv_hash_value_nolen_c(value, key);
    if (src == NULL) {
        if (is_req) {
            TEST_PRINTLN("Required parameter missing: %s", key);
            return LXB_STATUS_ERROR;
        }

        return LXB_STATUS_OK;
    }

    if (!unit_kv_is_bool(src)) {
        TEST_PRINTLN("Parameter '%s' must be a BOOL", key);
        return LXB_STATUS_ERROR;
    }

    *isas = unit_kv_bool(src);

    return LXB_STATUS_OK;
}

static lxb_status_t
test_value_encoding(unit_kv_value_t *value, lxb_encoding_t *enc)
{
    unit_kv_value_t *src;
    lexbor_str_t *str;
    const lxb_encoding_data_t *endata;

    *enc = LXB_ENCODING_UTF_8;

    src = unit_kv_hash_value_nolen_c(value, "encoding");
    if (src == NULL) {
        return LXB_STATUS_OK;
    }

    if (!unit_kv_is_string(src)) {
        TEST_PRINTLN("Parameter 'encoding' must be a STRING");
        return LXB_STATUS_ERROR;
    }

    str = unit_kv_string(src);

    endata = lxb_encoding_data_by_name(str->data, str->length);
    if (endata == NULL) {
        TEST_PRINTLN("Parameter 'encoding' contains an unsupported encoding");
        return LXB_STATUS_ERROR;
    }

    *enc = endata->encoding;

    return LXB_STATUS_OK;
}

static lxb_status_t
test_check_str(lexbor_str_t *have, const lexbor_str_t *need, const char *name)
{
    if (have->data == NULL && need->data == NULL) {
        return LXB_STATUS_OK;
    }

    if (have->length != need->length
        || (have->data == NULL && need->data != NULL)
        || (have->data != NULL && need->data == NULL)
        || memcmp(have->data, need->data, need->length) != 0)
    {
        printf("%s: not equal\n", name);

        if (have->data != NULL) {
            printf("    Have: %.*s\n", (int) have->length,
                   (const char *) have->data);
        }
        else {
            printf("    Have: NULL\n");
        }

        if (need->data != NULL) {
            printf("    Need: %.*s\n", (int) need->length,
                   (const char *) need->data);
        }
        else {
            printf("    Need: NULL\n");
        }

        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    printf("%s: ok\n", name);

    return LXB_STATUS_OK;
}
