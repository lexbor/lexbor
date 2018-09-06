# lexbor — HTML renderer

[![Build Status](https://travis-ci.org/lexborisov/lexbor.svg?branch=master)](https://travis-ci.org/lexborisov/lexbor)
<a href="https://scan.coverity.com/projects/lexborisov-lexbor">
    <img alt="Coverity Scan Build Status" src="https://scan.coverity.com/projects/16691/badge.svg"/>
</a>

The `lexbor` is being developed using the C language, without dependencies, and will benefit from the following unique features:

* Easy to port to any platform
* Embeddable, binding for third party programming languages
* Speed
* Full specifications support

## Features

* No outside dependencies

Available modules:
* Core
* [DOM](https://dom.spec.whatwg.org/)
* [HTML](https://html.spec.whatwg.org/multipage/)

In the plans:
* [Encoding](https://encoding.spec.whatwg.org/)
* [CSS](https://drafts.csswg.org/)
    * [Syntax]
    * [CSSOM], [CSSOM-View]
    * Modules: [Selectors], [Media], [Font] and so on
* [URL](https://url.spec.whatwg.org/)
* [Font](https://docs.microsoft.com/ru-ru/typography/opentype/spec/)
* Layout
* …
* Rendering engine
* …
* Browser

## Build and Installation

Please, see [INSTALL.md](https://github.com/lexborisov/lexbor/blob/master/INSTALL.md)

## External Bindings and Wrappers

You can create a binding or wrapper for the `lexbor` and place the link here!

## Documentation

Coming soon.

## Getting Help

* IRC: [#lexbor on `irc.freenode.net <http://freenode.net>`](http://webchat.freenode.net?channels=%23lexbor)
* E-mail [lex.borisov@gmail.com](mailto:lex.borisov@gmail.com)

## AUTHOR

Alexander Borisov <lex.borisov@gmail.com>

## COPYRIGHT AND LICENSE

   Lexbor.

   Copyright 2018 Alexander Borisov

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


See [LICENSE](https://github.com/lexborisov/lexbor/blob/master/LICENSE) file.


[Syntax]: https://drafts.csswg.org/css-syntax-3/
[CSSOM]: https://drafts.csswg.org/cssom-1/
[CSSOM-View]: https://drafts.csswg.org/cssom-view-1/
[Selectors]: https://drafts.csswg.org/selectors-4/
[Media]: https://drafts.csswg.org/mediaqueries-4/
[Font]: https://drafts.csswg.org/css-fonts-3/