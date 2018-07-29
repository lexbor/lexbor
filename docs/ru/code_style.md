# Общее положение


### Основное правило

Код проекта должен быть понятным и легко читаемым. Конечно, всё это относительно, но нужно стараться придерживаться этому правилу. Не нужно мудрить, нам поддерживать этот код.


### Отступы

* 4 пробела. Без табов!
* Страница из 80 столбцов. Допускается выход за границу. Всё что не помещается в границу 80 столбцов должно быть перенесено на новую строку (смотрите пример ниже).
* Не допускаются висящие пробелы. Если что-то разделяется новой строкой то она должна быть ПУСТОЙ.
* Перед `return`, `goto` и  `break` пустая строка.
* Перед и после `goto lable` пустая строка.

Выход за границу 80 столбцов:
1. Объявление функции с аргументами не помещаются в границу:
```C
static const lxb_char_t *
lxb_html_tokenizer_state_doctype_before_public_identifier(lxb_html_tokenizer_t *tkz,
                                                          const lxb_char_t *data,
                                                          const lxb_char_t *end);
```

2. При ветвлении
```C
    if (token != NULL) {
        /* ... */
        if (status != LXB_STATUS_OK) {
            lxb_html_tokenizer_error_add(tkz->parse_errors, tkz->token->end,
                                         LXB_HTML_TOKENIZER_ERROR_UNCHINUNATVAFOREXAM);
        }
    }
```

3. При объявлении переменной с присваиванием:

**Плохой стиль:**
```C
static const lxb_char_t *res = lxb_html_tokenizer_state_doctype_before_public_identifier(tkz,
                                                                                         data, 
                                                                                         end);
```

**Хороший стиль:**
```C
static const lxb_char_t *res;

res = lxb_html_tokenizer_state_doctype_before_public_identifier(tkz,
                                                                data, end);
```


### Переменные

* Все переменные объявляются в начале функции если функция не большая. Если функция большая то лучше объявлять переменные по месту
* Разрешается сразу присваивать значения объявленной переменной
* Если переменная используется только внутри определенного блока то она должна быть объявлена внутри этого блока

**Хороший стиль:**

```C
void
lxb_test(void)
{
    int a;
    long b = 0;
    size_t c;

    lxb_html_t *html;
    lxb_html_tree_t *html_tree;

    html = lxb_html_create(html);
    if (html == NULL) {
        /* ... */
    }

    html_tree = lxb_html_tree_create(html);
    if (html_tree == NULL) {
        /* ... */
    }
}
```

```C
lxb_status_t
lxb_test_two(void)
{
    lxb_html_tree_t *html_tree = lxb_get_blobal_context();

    if (html_tree == NULL) {
        return LXB_STATUS_ERROR;
    }

    return html_tree->call_best_function(1);
}
```

```C
lxb_status_t
lxb_test_two(void)
{
    lxb_html_tree_t *html_tree = lxb_get_blobal_context();

    if (html_tree == NULL) {
        lxb_status_t rc = lxb_get_global_error();

        printf("Status: %d\n", rc);

        return rc;
    }

    return html_tree->call_best_function(1);
}
```

```C
lxb_status_t
lxb_test_two(void)
{
    lxb_status_t status;
    lexbor_array_t *array;

    array  = lexbor_array_create();
    status = lexbor_array_init(array, 1024);

    if (status != LXB_STATUS_OK) {
        printf("Status: %d\n", status);
    }

    return status;
}
```

```C
lxb_status_t
lxb_test_two(void)
{
    lxb_status_t status = lexbor_array_init(lexbor_array_create(), 1024);

    if (status != LXB_STATUS_OK) {
        printf("Status: %d\n", status);
    }

    return status;
}
```


### Операторы if, if else, else

* Оператор от условия разделяется пробелом
* Последующие условия пишутся на новой строке
* Присваивание в условиях недопустимы

**Хороший стиль:**

```C
lxb_status_t status;

/* ... */

if (status != LXB_STATUS_OK) {
    return status;
}
```

```C
if (status == LXB_STATUS_SOME) {
    return 101;
}
else if (status == LXB_STATUS_SOME_TWO) {
    return 1002;
}
else {
    printf("Зачем тут else?!");
}
```

```C
status = (object != NULL) ? 0 : 1;
```

```C
status = function_name(blah, blah);
if (status != LXB_STATUS_OK) {
    return status;
}
```

```C
if (status == LXB_STATUS_SOME
    || status == LXB_STATUS_SOME_TWO) 
{
    return 101;
}
```

**Плохой стиль:**

```C
if ((status = function_name(blah, blah))) {
    return status;
}
```


### Циклы

* Цикл от условия разделяется пробелом
* Можно объявлять переменную в for
* Можно переносить открывающую фигурную скобку на новую строку

**Хороший стиль:**

```C
while (1) {
    printf("God, bless this loop!\n");
}
```

```C
for (size_t i = 0; i < 1000; i++) {
    printf("Oh God\n");
}
```

```C
do {
    printf("Oh no, oh no, oh no no no!\n");
}
while (1);
```

```C
for (size_t i = 0; i < 1000; i++) {
    some_big_name_of_great_function(and_arg, and_arg);
    ...
}
```

```C
for (some_long_long_name = 0; 
     some_long_long_name < dome_funct(123); 
     some_long_long_name++) 
{
    while (i < some_var_to) {
        
    }
}
```


### include

* После `#include` обязательно две новые строки
* Если файл находится в директории `/source/*` то для включения используем кавычки `#include "lexbor/html/tree.h"`


### Структуры

* Объявление структур отделяется двумя новыми строками от всего остального
* Все переменные выравниваются по самому длинному имени типа

Два подхода для объявления структур:

**Первый**
```C
typedef struct {
    /* Some */
}
lxb_<имя-модуля>_<файл>_<название>_t;
```

**Второй**
```C
typedef lxb_<имя-модуля>_<файл>_<название> lxb_<имя-модуля>_<файл>_<название>_t;

struct lxb_<имя-модуля>_<файл>_<название> {
    /* Some */
    lxb_<имя-модуля>_<файл>_<название>_t *my;
};
```

**Пример**
```C
typedef struct {
    const lxb_char_t      *begin;
    const lxb_char_t      *end;

    lexbor_in_node_t      *in_begin;

    lxb_html_token_attr_t *attr_first;
    lxb_html_token_attr_t *attr_last;

    void                  *parent_element;

    lxb_html_tag_id_t     tag_id;
    lxb_html_token_type_t type;
}
lxb_html_token_t;

```


### Преоброзование типов

* Разделается пробелом переменная и тип к которому приводим

**Хороший стиль:**

```C
lxb_status_t status = (lxb_status_t) our_status;
```

```C
lxb_html_tree_t *tree = (lxb_html_tree_t *) our_status;
```


### Комментарии

* Используется только Си стиль комментариев `/* comment */`
* Комментарии пишутся правильно. С заглавной буквы и точкой на конце если комментарий больше оного предложения
* В начале каждой новой строки комментария ставим символ '*'
* Комментарии только на английском языке

**Хороший стиль:**

```C
/* This is Style */
```

```C
/* 
 * This is Style.
 */
```


### Функции

* Первой строкой идёт тип
* Вторая строка имя функции и аргументы
* Блок кода на последующих строках
* Аргументы переносятся за открывающую скобку

**Хороший стиль:**

```C
lexbor_array_t * 
lexbor_array_create(void)
{
    /* Code */
}
```

```C
lexbor_array_t * 
lexbor_array_create(int arg1, void *arg2, 
                    char *arg3, size_t len)
{
    /* Code */
}
```


### Именование функций

Для создания имени функции используется шаблон:<br>
`lxb_<название-модуля>_<путь-к-файлу>_<название-файла>_<название>(...);`

Для примера возмём функцию `lxb_html_tree_create(...)`:<br>
Найти её легко в `/source/lexbor/html/tree.c`

Исключением является основной модуль (`lexbor/core`) проекта `lexbor`:<br>
`lexbor_<название-файла>_<название>(...);`

Иными словами, если в проекте встречается `lexbor_*` то это означает, что речь идет именно об основном модуле `lexbor` — `core`.

Если функции объявлены как `static` то они могут иметь укороченное именование, но обязательно должен присутствовать префикс: `lxb_<название-модуля>_`.
Например, есть файл `lexbor/html/tree/insertion_mode/in_table.c` у него есть функция `lxb_html_tree_clear_stack_back_to_table_context` объявленная как `static`.
Если бы мы использовали описанный выше подход с полным именованием функции используя, весь путь до файла, то имя получилось бы слишком большим:
`lxb_html_tree_insertion_mode_in_table_clear_stack_back_to_table_context` — это очень не удобно.


### inline функции

* Всегда находятся в заголовочных файлах в самом конце
* Перед группой инлайн функций вставляется комментарий `Inline functions`
* Всегда ничинаются с `lxb_inline`

**Пример**
```C
/*
 * Inline functions
 */
lxb_inline void
lxb_html_token_clean(lxb_html_token_t *token)
{
    memset(token, 0, sizeof(lxb_html_token_t));
}
```


### Функции Конструкторы/Деструкторы

Для API создания, инициализации, очистки, удаления объектов используется следующий шаблон:

```C
<имя-структуры> *
<префикс-функции>_create(void);

lxb_status_t
<префикс-функции>_init(<имя-структуры>* obj);

void
<префикс-функции>_clean(<имя-структуры>* obj);

void
<префикс-функции>_clean_all(<имя-структуры>* obj);

<имя-структуры> *
<префикс-функции>_destroy(<имя-структуры>* obj, bool self_destroy);
```

Функция инициализации объекта `*_init` может принимать любое количество аргументов и должна всегда возвращать `lxb_status_t`.
Функции очистки `*_clean` `*_clean_all` могут возвращать любое значение, обычно void.

Если в функцию инициализации объекта `*_init` передать `NULL` в качестве первого аргумента (объект) то функция должна вернуть статус `LXB_STATUS_ERROR_OBJECT_NULL`.

Если при вызове функции `*_destroy` аргумент `self_destroy` указан как `true` то возвращаемое значение всегда должно быть равно `NULL`, иначе возвращается переданный в аргументах объект `obj`. Данный подход был реализован для работы с динамически выделенными объектами и теми что находятся в стеке (статические).

Если функция `*_destroy` не имеет аргумента `bool self_destroy` значит объект может быть создан только при помощи функции `*_create` (не на стеке).

Пример с динамически выделенным объектом:

```C
lexbor_array_t *array = lexbor_array_create();
lxb_status_t status = lexbor_array_init(array, 1024);

if (status != LXB_STATUS_OK) {
	exit(EXIT_FAILURE);
}

lexbor_array_destroy(array, true);
```

Пример с объектом из стека:

```C
lexbor_array_t array = {0};
lxb_status_t status = lexbor_array_init(&array, 1024);

if (status != LXB_STATUS_OK) {
	exit(EXIT_FAILURE);
}

lexbor_array_destroy(&array, false);
```

Пример API инициализации для ARRAY:

```C
lexbor_array_t *
lexbor_array_create(void);

lxb_status_t
lexbor_array_init(lexbor_array_t *array, size_t start_size);

void
lexbor_array_clean(lexbor_array_t *array);

lexbor_array_t *
lexbor_array_destroy(lexbor_array_t *array, bool self_destroy);
```

Стоит отметить, что данный подход не является абсолютным постулатом. Бывают случаи когда приходится реализовывать иное API, но всё же, в большинстве случаев именно такое. Нужно быть разумным и понимать что делаешь, а не слепо следовать правилам.


### Исходные файлы и глобальные переменные

В проекте заведено так, что каждый файл `*.h` и `*.c` отвечает за свою область.

**Плохо** когда исходный файл содержит в себе реализации функций для работы с несколькими структурами/объектами по смыслу никак не схожие между собой.

**Файлы с данными**

Не желательно вставлять данные (глобальные переменные, "ресурсы") в один файл с описанием функций, структур. Так же не желательно вставлять ресурсы в исходный код `*.c`. Всё это может вызвать проблемы.
Для ресурсов создается отдельный файл заголовков `*.h` в котором находятся только нужные ресурсы. И включается данный файл только в исходник `*.c`.

Например:

```C
/* файл /source/lexbor/resources.h */

#ifndef LEXBOR_RESOURCES_H
#define LEXBOR_RESOURCES_H

static const unsigned char lexbor_string_map_hex_to_char[] = {
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x00
};

#endif /* LEXBOR_RESOURCES_H */
```

```C
/* файл /source/lexbor/mystring.c */

#include "lexbor/resources.h"

void 
lexbor_string_some(void) 
{
	printf("%c\n", lexbor_map_string_hex_to_char[0]);
}
```


### Заголовочные файлы

Все пути указываются относительно `/source/` директории. Для примера, если нам нужно подключить заголовок от модуля `html` который находится в каталоге `/source/lexbor/html/`: `#include "lexbor/html/tree.h"`

При подключении любых внутренних модулей используются кавычки (U+0022 QUOTATION MARK (")).


### Шаблон для всех *.h и *.c файлов

**\*.h**

```C
/*
 Copyright (C) <текущий год цифрами> Alexander Borisov

 Author: Имя Фамилия <<почта>@<автора>>
*/

#ifndef LEXBOR_<ПУТЬ-К-ФАЙЛУ>_<НАЗВАНИЕ-ФАЙЛА>_H
#define LEXBOR_<ПУТЬ-К-ФАЙЛУ>_<НАЗВАНИЕ-ФАЙЛА>_H

#ifdef __cplusplus
extern "C" {
#endif

/* All includes */

/* Some code */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif LEXBOR_<ПУТЬ-К-ФАЙЛУ>_<НАЗВАНИЕ-ФАЙЛА>_H
```

**\*.c**

```C
/*
 Copyright (C) 2018 Alexander Borisov

 Author: Имя Фамилия <<почта>@<автора>>
*/

#include "<путь к заголовку>.h"


/* Code */
```
