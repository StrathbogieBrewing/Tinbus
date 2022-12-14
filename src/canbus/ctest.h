#ifndef CTEST_H
#define CTEST_H
    void ctest_result(const char* name, int line); 
    #ifdef CTEST_ENABLED
        #define CTEST_DO_STRINGIFY(x) #x
        #define CTEST_STRINGIFY(s) CTEST_DO_STRINGIFY(s)
        #define CTEST_RUN(function) do { \
            void ctest_##function##_unit_test(const char* name); \
            ctest_##function##_unit_test(CTEST_STRINGIFY(function)); } while (0)
        #define CTEST_FUNCTION(function) \
            void ctest_##function##_unit_test(const char* name)
        #define CTEST_ASSERT(assertion) do { if (assertion) ctest_result(name, __LINE__); else ctest_result(name, -(__LINE__));  } while (0)
    #else   // CTEST_ENABLED
        #define CTEST_RUN(function)
        #define CTEST_FUNCTION(function) static inline void ctest_##function##_unit_test(const char* name)
        #define CTEST_ASSERT(assertion)
    #endif  // CTEST_ENABLED
#endif  // CTEST_H
