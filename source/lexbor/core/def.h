/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_DEF_H
#define LEXBOR_DEF_H

#define LEXBOR_STRINGIZE_HELPER(x) #x
#define LEXBOR_STRINGIZE(x) LEXBOR_STRINGIZE_HELPER(x)

/* Format */
#ifdef _MSC_VER
    #define LEXBOR_FORMAT_Z "%Iu"
#else
    #define LEXBOR_FORMAT_Z "%zu"
#endif

/* Deprecated */
#ifdef _MSC_VER
    #define LEXBOR_DEPRECATED(func) __declspec(deprecated) func
#elif defined(__GNUC__) || defined(__INTEL_COMPILER)
    #define LEXBOR_DEPRECATED(func) func __attribute__((deprecated))
#else
    #define LEXBOR_DEPRECATED(func) func
#endif

/* Debug */
//#define LEXBOR_DEBUG(...) do {} while (0)
//#define LEXBOR_DEBUG_ERROR(...) do {} while (0)

#define LEXBOR_MEM_ALIGN_STEP sizeof(void *)

#endif /* LEXBOR_DEF_H */

