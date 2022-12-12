#ifndef CTEST_H
#define CTEST_H
    #ifdef CTEST_ENABLED
        #define CTEST_DO_STRINGIFY(x) #x
        #define CTEST_STRINGIFY(s) CTEST_DO_STRINGIFY(s)
        #define ctest_run(function) do { \
            int ctest_##function##_unit_test(void); \
            int line = ctest_##function##_unit_test(); \
            if(line != 0) CTEST_ERROR(CTEST_STRINGIFY(function), line); } while(0)
        #define ctest_function(function) \
            static int line = 0; \
            static inline void ctest_##function##_function(int *line); \
            int ctest_##function##_unit_test(void) { ctest_##function##_function(&line); return line; } \
            static inline void ctest_##function##_function(int *line)
        #define ctest_assert(assertion) do { if (!assertion) { *line = __LINE__; return; } } while (0)
    #else   // CTEST_ENABLED
        #define ctest_run(function)
        #define ctest_function(function) static inline void ctest_##function##_function(int *line)
        #define ctest_assert(assertion)
    #endif  // CTEST_ENABLED

#endif  // CTEST_H
