#ifndef CTEST_H
#define CTEST_H

#include <stdbool.h>
#include <stdint.h>

#define DO_STRINGIFY(x) #x
#define STRINGIFY(s) DO_STRINGIFY(s)

struct ctest_unit_test {
    const char *name;
    int (*test)(void);
    int line;
};

#ifndef __aligned
#define __aligned(x)	__attribute__((__aligned__(x)))
#endif

#define __used		__attribute__((__used__))

#ifndef __noasan
#define __noasan /**/
#endif

#define ___in_section(a, b, c) \
	__attribute__((section("." DO_STRINGIFY(a)			\
				"." DO_STRINGIFY(b)			\
				"." DO_STRINGIFY(c))))
#define __in_section(a, b, c) ___in_section(a, b, c)

#define Z_DECL_ALIGN(type) __aligned(__alignof(type)) type
#define STRUCT_SECTION_ITERABLE(struct_type, name) \
	Z_DECL_ALIGN(struct struct_type) name \
	__in_section(_##struct_type, static, name) __used __noasan



// #define Z_DECL_ALIGN(type) __aligned(__alignof(type)) type
// #define STRUCT_SECTION_ITERABLE(struct_type, name)  Z_DECL_ALIGN(struct struct_type) name __in_section(_##struct_type, static, name) __used __noasan

// static __aligned(__alignof(ctest_unit_test)) ctest_unit_test __in_section(_##struct_type, static, name) __used
// __noasan

#define ctest_function(function)                                                                               \
    static int __line__ = 0;  \
    int function##_wrapper(void); \
    static STRUCT_SECTION_ITERABLE(ctest_unit_test, function##_unit_test) = { .name = STRINGIFY(function), .test = (function##_wrapper)};     \
    static void function##_test();   \
    int function##_wrapper(void) { function##_test();  return __line__; }        \
    static inline void function##_test()

// static void name##_wrapper(void); 
    // int name##_wrapper(void);                                                                                   

// static void _##name##_unit_test(void *data); 
    // struct ctest_unit_test ctest_unit_test__##name = { .name = STRINGIFY(name), .test = (_##name##_unit_test)}; 
    // static void _##name##_unit_test(void *data)  { }  
	// static inline void name##_()

// COND_CODE_1(use_fixture, (struct suite##_fixture *fixture), (void)))
// struct ctest_unit_test ctest_unit_test__##fn = { .name = STRINGIFY(name), .test = (_##name##_unit_test)};
// static void _##name##_unit_test(void *data)  { }

#define ctest_assert(assertion) do { if (!assertion) { __line__ = __LINE__; return; } } while (0)


#endif // CTEST_H

// #define TEST_XSTR(a) TEST_STR(a)
// #define TEST_STR(a) #a
// #define TEST_ASSERT(test) do { if (!(test)) return ("Fail " __FILE__ ":" TEST_XSTR(__LINE__)); } while (0)
// #define TEST_RUN(test) do { char *message = test(); if (message) return message; } while (0)

// char * hex_unit_tests(void){
//     TEST_ASSERT(hex_to_u4('x').error);
//     TEST_ASSERT(hex_to_u4('a').value == 10);
//     TEST_ASSERT(hex_to_u4('a').value == 11);
//     return 0;
// }

// #endif // TEST_BUILD