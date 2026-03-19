/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/style/html/interfaces/option_element.h"
#include "lexbor/style/style.h"

LXB_API lxb_status_t
lxb_html_option_attr_steps_change(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns);

LXB_API lxb_status_t
lxb_html_option_attr_steps_remove(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns);


lxb_status_t
lxb_style_html_option_attr_change(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns)
{
    lxb_status_t status;

    status = lxb_html_option_attr_steps_change(element, name, old_value,
                                               old_len, value, value_len, ns);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return lxb_style_html_element_attr_change(element, name, old_value,
                                              old_len, value, value_len, ns);
}

lxb_status_t
lxb_style_html_option_attr_append(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns)
{
    lxb_status_t status;

    status = lxb_html_option_attr_steps_change(element, name, old_value,
                                               old_len, value, value_len, ns);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return lxb_style_html_element_attr_append(element, name, old_value,
                                              old_len, value, value_len, ns);
}

lxb_status_t
lxb_style_html_option_attr_remove(lxb_dom_element_t *element,
                                  lxb_dom_attr_id_t name,
                                  const lxb_char_t *old_value, size_t old_len,
                                  const lxb_char_t *value, size_t value_len,
                                  lxb_ns_id_t ns)
{
    lxb_status_t status;

    status = lxb_html_option_attr_steps_remove(element, name, old_value,
                                               old_len, value, value_len, ns);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return lxb_style_html_element_attr_remove(element, name, old_value,
                                              old_len, value, value_len, ns);
}

lxb_status_t
lxb_style_html_option_attr_replace(lxb_dom_element_t *element,
                                   lxb_dom_attr_id_t name,
                                   const lxb_char_t *old_value, size_t old_len,
                                   const lxb_char_t *value, size_t value_len,
                                   lxb_ns_id_t ns)
{
    return lxb_style_html_element_attr_replace(element, name, old_value,
                                               old_len, value, value_len, ns);
}
