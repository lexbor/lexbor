/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 *
 * Convert html tag tree into s-expression string in stdout
 */

#include <lexbor/core/fs.h>

#include "base.h"


static void
usage(void);

static lxb_status_t
tree_walker(lxb_dom_node_t *node, lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
attributes(lxb_dom_node_t *node, lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
serialize_cb(const lxb_char_t *data, size_t len, void *ctx);


static void
usage(void)
{
    fprintf(stderr, "html2sexpr <file>\n");
}

int
main(int argc, const char *argv[])
{
    if (argc != 2) {
        usage();
        FAILED("Invalid number of arguments");
    }

    lxb_status_t status;
    lxb_html_document_t *document;
    lxb_char_t *html;
    size_t html_len;

    html = lexbor_fs_file_easy_read((const lxb_char_t *) argv[1], &html_len);
    if (html == NULL) {
        FAILED("Failed to read HTML file");
    }

    /* Initialization */
    document = lxb_html_document_create();
    if (document == NULL) {
        PRINT("Failed to create HTML Document");
        goto failed;
    }

    /* Parse */
    status = lxb_html_document_parse(document, html, html_len);
    if (status != LXB_STATUS_OK) {
        PRINT("Failed to parse HTML");
        goto failed;
    }

    status = tree_walker(lxb_dom_interface_node(document)->first_child,
                         serialize_cb, NULL);
    if (status != LXB_STATUS_OK) {
        PRINT("Failed to convert HTML to S-Expression");
        goto failed;
    }

    /* Destroy */
    lxb_html_document_destroy(document);
    lexbor_free(html);

    return EXIT_SUCCESS;

failed:

    lxb_html_document_destroy(document);
    lexbor_free(html);

    return EXIT_FAILURE;
}

static lxb_status_t
tree_walker(lxb_dom_node_t *node, lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    lxb_html_template_element_t *temp;
    lxb_dom_node_t *root = node->parent;

    const lxb_char_t *name;
    size_t name_len = 0;

    bool skip_it;

    while (node != NULL) {
        if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
            status = cb((const lxb_char_t *) "(", 1, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            name = lxb_dom_element_qualified_name(lxb_dom_interface_element(node),
                                                  &name_len);

            status = cb(name, name_len, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            status = attributes(node, cb, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            if (node->local_name == LXB_TAG_TEMPLATE) {
                temp = lxb_html_interface_template(node);

                if (temp->content != NULL) {
                    if (temp->content->node.first_child != NULL)
                    {
                        status = tree_walker(&temp->content->node, cb, ctx);
                        if (status != LXB_STATUS_OK) {
                            return status;
                        }
                    }
                }
            }

            skip_it = false;
        }
        else {
            skip_it = true;
        }

        if (skip_it == false && node->first_child != NULL) {
            node = node->first_child;
        }
        else {
            while(node != root && node->next == NULL)
            {
                if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                    status = cb((const lxb_char_t *) ")", 1, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }

                node = node->parent;
            }

            if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                status = cb((const lxb_char_t *) ")", 1, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }
            }

            if (node == root) {
                break;
            }

            node = node->next;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
attributes(lxb_dom_node_t *node, lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    const lxb_char_t *data;
    size_t data_len;

    attr = lxb_dom_element_first_attribute(lxb_dom_interface_element(node));

    while (attr != NULL) {
        status = cb((const lxb_char_t *) "(", 1, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        data = lxb_dom_attr_qualified_name(attr, &data_len);

        status = cb(data, data_len, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        data = lxb_dom_attr_value(attr, &data_len);

        if (data != NULL) {
            status = cb((const lxb_char_t *) " '", 2, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            status = cb(data, data_len, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            status = cb((const lxb_char_t *) "'", 1, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }
        }

        status = cb((const lxb_char_t *) ")", 1, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        attr = lxb_dom_element_next_attribute(attr);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
serialize_cb(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}
