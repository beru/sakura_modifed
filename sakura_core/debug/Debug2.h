#pragma once

// assertŠÖ”

#ifdef assert
#undef assert
#endif

#ifdef _DEBUG

void debug_output(const char* str, ...);
void debug_exit();
void warning_point();

#define assert(exp) \
{ \
	if (!(exp)) { \
		debug_output("!assert: %hs(%d): %hs\n", __FILE__, __LINE__, #exp); \
		debug_exit(); \
	} \
}

#define assert_warning(exp) \
{ \
	if (!(exp)) { \
		debug_output("!warning: %hs(%d): %hs\n", __FILE__, __LINE__, #exp); \
		warning_point(); \
	} \
}

#define ASSERT_GE(a, b) assert(a >= b)

#else // #ifdef _DEBUG

#define assert(exp)
#define assert_warning(exp)
#define ASSERT_GE(a, b)
#define NON_NEGATIVE(a)

#endif // #ifdef _DEBUG

