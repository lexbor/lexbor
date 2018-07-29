# Токены


### !DOCTYPE

У данного вида токена есть своя особенность хранения информации.

Возьмём для примера:
```HTML
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
```

После парсинга DOCTYPE из примера будет создан токен с тремя атрибутами:
```
1. name = HTML;
2. name = PUBLIC; value = -//W3C//DTD HTML 4.01 Transitional//EN
3. value = http://www.w3.org/TR/html4/loose.dtd
```

У первого атрибута отсутствует значение `value`, а последнего отсутствует `name`.

Если у второго и третьего атрибута значение `value_begin` и `value_end` равно `NULL` значит оно не представлено.
Тут стоит отметить, что значение считается представленным даже есть открылась кавычка. Например:

```HTML
<!DOCTYPE HTML PUBLIC "
```

Из примера выше будет создан токен с двумя атрибутами:
```
1. name = HTML;
2. name = PUBLIC; value = 
```

У второго атрибута значение `value_begin` и `value_end` не будет равно `NULL`, а значит оно представлено.
