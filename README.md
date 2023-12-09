# Lexbor

The `lexbor` project is being developed using the `C` language, without dependencies.

## Features
* [Modules](https://github.com/lexbor/lexbor/tree/master/source/lexbor).
* [Single or separate](https://github.com/lexbor/lexbor#single-or-separately) libraries for each module.
* No outside dependencies.
* Easy to port to any platform.
* C99 support.
* Speed.

### HTML Module
* Full conformance with the [HTML5 specification](https://html.spec.whatwg.org/multipage/).
* Manipulation of [elements](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/element_create.c) and [attributes](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/element_attributes.c): add, change, delete and other.
* Supports fragment parsing (for [innerHTML](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/element_innerHTML.c)).
* Supports parsing [by chunks](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/document_parse_chunk.c).
* Passes all tree construction tests.
* [Tested](https://github.com/lexbor/warc_test) by 200+ million HTML pages with [ASAN](https://clang.llvm.org/docs/AddressSanitizer.html).
* Two way for parsing HTML: [by Document](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/document_parse.c), [by Parser](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/parse.c).
* Supports [determining encoding](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/encoding.c) by byte stream.
* [Parsing CSS Styles](https://github.com/lexbor/lexbor/tree/master/examples/lexbor/styles) in tag attributes and in the `<style>` tag.
* Fast.

### CSS Module
* Full conformance with the [CSS Syntax](https://drafts.csswg.org/css-syntax-3/) module.
* Supports:
* * [x] [Selectors](https://github.com/lexbor/lexbor/tree/master/examples/lexbor/selectors).
* * [x] [StyleSheet Tree](https://github.com/lexbor/lexbor/tree/master/examples/lexbor/css) (aka CSSOM).
* * [x] and so on.
* Fast.

Supported CSS Properties for StyleSheet/Declarations:
```
align-content, align-items, align-self, alignment-baseline, background-color,
baseline-shift, baseline-source, border, border-bottom, border-bottom-color,
border-left, border-left-color, border-right, border-right-color, border-top,
border-top-color, bottom, box-sizing, clear, color, direction, display,
dominant-baseline, flex, flex-basis, flex-direction, flex-flow, flex-grow,
flex-shrink, flex-wrap, float, float-defer, float-offset, float-reference,
font-family, font-size, font-stretch, font-style, font-weight,
hanging-punctuation, height, hyphens, inset-block-end, inset-block-start,
inset-inline-end, inset-inline-start, justify-content, left, letter-spacing,
line-break, line-height, margin, margin-bottom, margin-left, margin-right,
margin-top, max-height, max-width, min-height, min-width, opacity, order,
overflow-block, overflow-inline, overflow-wrap, overflow-x, overflow-y,
padding, padding-bottom, padding-left, padding-right, padding-top, position,
right, tab-size, text-align, text-align-all, text-align-last, text-combine-upright,
text-decoration, text-decoration-color, text-decoration-line, text-decoration-style,
text-indent, text-justify, text-orientation, text-overflow, text-transform, top,
unicode-bidi, vertical-align, visibility, white-space, width, word-break,
word-spacing, word-wrap, wrap-flow, wrap-through, writing-mode, z-index
```

Properties that are unknown to the parser will be created as custom (`lxb_css_property__custom_t`).
Support for new properties is added regularly.

### Encoding Module
* Full conformance with the [Encoding specification](https://encoding.spec.whatwg.org/).
* Supports `40 encodings` for encode/decode.
* Supports [single](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/encoding/single/from_to.c) and [buffering](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/encoding/buffer/from_to.c) encode/decode.
* Fast.

## Build and Installation

### Binary packages

Binaries are available for:

* [CentOS](https://lexbor.com/download/#centos) 6, 7, 8
* [Debian](https://lexbor.com/download/#debian) 8, 9, 10, 11
* [Fedora](https://lexbor.com/download/#fedora) 28, 29, 30, 31, 32, 33, 34, 36, 37
* [RHEL](https://lexbor.com/download/#rhel) 7, 8
* [Ubuntu](https://lexbor.com/download/#ubuntu) 14.04, 16.04, 18.04, 18.10, 19.04, 19.10, 20.04, 20.10, 21.04, 22.04

Currently for `x86_64` architecture.
If you need any other architecture, please, write to [support@lexbor.com](mailto:support@lexbor.com).

### vcpkg

For vcpkg users there is a `lexbor` [port](https://github.com/microsoft/vcpkg/tree/master/ports/lexbor) that can be installed via `vcpkg install lexbor` or by adding it to `dependencies` section of your `vcpkg.json` file.

### macOS

#### Homebrew

To install `lexbor` on macOS from Homebrew:

```sh
brew install lexbor
```

#### MacPorts

To install `lexbor` on macOS from MacPorts:

```sh
sudo port install lexbor
```

### Source code

For building and installing Lexbor library from source code, use [CMake](https://cmake.org/) (open-source, cross-platform build system).

```bash
cmake . -DLEXBOR_BUILD_TESTS=ON -DLEXBOR_BUILD_EXAMPLES=ON
make
make test
```

Please, see more information in [documentation](https://lexbor.com/docs/lexbor/#source_code).

## Single or separately

### Single
* liblexbor — this is a single library that includes all modules.

### Separately
* liblexbor-{module name} — libraries for each module.

You only need an HTML parser? Use `liblexbor-html`.

Separate modules may depend on each other.
For example, dependencies for `liblexbor-html`: `liblexbor-core`, `liblexbor-dom`, `liblexbor-tag`, `liblexbor-ns`.

The `liblexbor-html` library already contains all the pointers to the required dependencies. Just include it in the assembly: `gcc program.c -llexbor-html`.

## External Bindings and Wrappers

* [Elixir](https://git.pleroma.social/pleroma/elixir-libraries/fast_html) binding for the HTML module (since 2.0 version)
* [Crystal](https://github.com/kostya/lexbor) Fast HTML5 Parser with CSS selectors for Crystal language
* [Python](https://github.com/rushter/selectolax#available-backends) binding for modest and lexbor engines.
* [D](https://github.com/trikko/parserino) Fast HTML5 Parser with CSS selectors for D programming language
* [Ruby](https://github.com/serpapi/nokolexbor) Fast HTML5 Parser with both CSS selectors and XPath support.

You can create a binding or wrapper for the `lexbor` and place the link here!

## Documentation

Available on [lexbor.com](https://lexbor.com) in [Documentation](https://lexbor.com/docs/lexbor/) section.

## Roadmap

Please, see [roadmap](https://lexbor.com/roadmap/) on [lexbor.com](https://lexbor.com).

## Getting Help

* E-mail [support@lexbor.com](mailto:support@lexbor.com)

## AUTHOR

Alexander Borisov <borisov@lexbor.com>

## COPYRIGHT AND LICENSE

   Lexbor.

   Copyright 2018-2023 Alexander Borisov

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


Please, see [LICENSE](https://github.com/lexbor/lexbor/blob/master/LICENSE) file.
