# Changelog

## [Unreleased]

## [2.1.0] - 2021-08-05
### Added
- CSS: parsing selectors.
- Selectors for find DOM/HTML nodes.
- Build: clang fuzzer support.

### Fixed
- Core: fixed includes in "core.h".
- DOM: fixed skip child nodes in simple walker.
- HTML: fixed the incorrect state of the switch for "pre", "listing", "textarea".
- HTML: fixed heap-buffer-overflow in active/open elements.

### Changed
- HTML: refactoring module for better performance.
- CSS: parsing api and token retrieval changed.

## [1.0.0] - 2020-03-13
### Added
- Core: added hash table implementation.
- Created public header file for all modules.

### Fixed
- HTML: memory leak of repeated parsing of document.
- NULL pointer use in lxb_dom_attr_compare().
- Symbols visibility for Windows.

### Changed
- DOM, HTML, Tag, NS: breaking API changes.
- DOM: node tag_id to local_name.
- DOM: attribute name now is uintptr_t. Reference to global unique naming.

## [0.4.0] - 2019-11-18
### Added
- Encoding module.
- Utils module.
- CMake option for build all modules separately.
- Examples for html tokenizer.
- HTML: prescan the byte stream to determine encoding function.
- Aliases for inline functions for use ABI of library.
- Support ASAN for memory pool.
- Core: added dup function for mraw.
- More statuses.
- Converting functions for string to number.

### Fixed
- Full path for cmake test command.
- HTML: fixed parse '<![[CDATA[' chunks.
- Use after free document element in fragment parse.
- HTML: fixed memory leak in tokenizer.
- HTML: fixed pointer offset for lxb_dom_node_text_content() function.
- HTML: fixed use-after-free after clearing a document.

### Changed
- Core: changed lexbor_str_length_set() function.

## [0.2.0] - 2019-03-12
### Added
- CSS:Syntax parser.
- Core: added convertation floating-point numbers from/to string.
- DOM: general implementation of the functional.

### Fixed
- HTML: fixed problem with serialize U+00A0 character. #22
- Fixed build with C++. #20

## [0.1.0] - 2018-11-30
### Added
- The Lexbor project.
- HTML Parser.
- HTML/DOM interfaces.
- Basic functions for DOM and HTML interfaces.
- Examples for HTML and DOM.
- Tests for Core module.
- Tests for HTML tokenizator and tree builder.
- Python scripts for generating static structures.
