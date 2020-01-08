# Changelog

## [Unreleased]

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
