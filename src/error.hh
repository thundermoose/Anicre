
#ifndef __ERROR_HH__
#define __ERROR_HH__

#include "location.hh"
#include "file_line.hh"
#include "util.hh"
#include "markconvbold.hh"
#include <stdio.h>

#include <stdarg.h>
#include <pthread.h>

#include <assert.h>

#ifdef __LAND02_CODE__
#ifndef UNUSED
#define UNUSED(x) ((void) x)
#endif
#endif

struct error
{

};

inline void print_error(error &e) { UNUSED(e); }


#if defined __GNUC__ && __GNUC__ < 3 // 2.95 do not do iso99 variadic macros
// We no longer want to have to handle the double definitions of all
// out variable argument macros.  Besides, gcc 2.95 also does not like
// our anonymous structures, used by ucesb generated code
#error This code is no longer compiling with gcc < 3
#endif

enum fe_msgtype
  {
    FE_UNKNOWN = 0,
    FE_ERROR = 1,
    FE_WARNING,
    FE_INFO,
  };

class formatted_error
{
public:
  void init(fe_msgtype type)
  {
    _type = type;
    _length = 0;
  }

public:
  fe_msgtype   _type;
  char        *_buffer;
  size_t       _length;
  size_t       _alloc;

protected:
  void realloc(size_t newlen);
  void make_message(const char *fmt,va_list ap);

public:
  void printf(const char *fmt,...)
    __attribute__ ((__format__ (__printf__, 2, 3)));
  void append(int c);

  void eject();
};

#ifdef USE_PTHREAD
#ifdef HAVE_THREAD_LOCAL_STORAGE
extern __thread formatted_error __fe;
#else
extern pthread_key_t _fe_key;
#define __fe (*((formatted_error*) pthread_getspecific(_fe_key)))
formatted_error &get_fe();
void fe_init();
#endif
#else
extern formatted_error __fe;
#endif

#define STDERR_BEGIN(type)  \
  formatted_error *__local_fe = &__fe; __local_fe->init(FE_##type);
#define STDERR_PRINTF(...)  __local_fe->printf(__VA_ARGS__)
#define STDERR_PUTC(c)      __local_fe->append(c)
#define STDERR_END          __local_fe->eject();
#define STDERR_FILE         (*__local_fe)


#if defined __GNUC__ && __GNUC__ < 3 // 2.95 do not do iso99 variadic macros
#define FATAL(__VA_ARGS__...) do { \
  STDERR_BEGIN(ERROR);         \
  /*STDERR_PRINTF("%s:%d: ",__FILE__,__LINE__);*/  \
  STDERR_PRINTF(__VA_ARGS__);  \
  STDERR_END;                  \
  exit(1);                     \
} while (0)
#else
#define FATAL(...) do {        \
  STDERR_BEGIN(ERROR);         \
  /*STDERR_PRINTF("%s:%d: ",__FILE__,__LINE__);*/  \
  STDERR_PRINTF(__VA_ARGS__);  \
  STDERR_END;                  \
  exit(1);                     \
} while (0)
#endif

#if defined __GNUC__ && __GNUC__ < 3 // 2.95 do not do iso99 variadic macros
#define ERROR(__VA_ARGS__...) do { \
  STDERR_BEGIN(ERROR);         \
  /*STDERR_PRINTF("%s:%d: ",__FILE__,__LINE__);*/  \
  STDERR_PRINTF(__VA_ARGS__);  \
  STDERR_END;                  \
  throw error();	       \
} while (0)
#else
#define ERROR(...) do {        \
  STDERR_BEGIN(ERROR);         \
  /*STDERR_PRINTF("%s:%d: ",__FILE__,__LINE__);*/  \
  STDERR_PRINTF(__VA_ARGS__);  \
  STDERR_END;                  \
  throw error();	       \
} while (0)
#endif

#if defined __GNUC__ && __GNUC__ < 3 // 2.95 do not do iso99 variadic macros
#define WARNING(__VA_ARGS__...) do { \
  STDERR_BEGIN(WARNING);       \
  /*STDERR_PRINTF("%s:%d: ",__FILE__,__LINE__);*/  \
  STDERR_PRINTF(__VA_ARGS__);  \
  STDERR_END;                  \
} while (0)
#else
#define WARNING(...) do {      \
  STDERR_BEGIN(WARNING);       \
  /*STDERR_PRINTF("%s:%d: ",__FILE__,__LINE__);*/  \
  STDERR_PRINTF(__VA_ARGS__);  \
  STDERR_END;                  \
} while (0)
#endif

#ifdef __LAND02_CODE__
#if defined __GNUC__ && __GNUC__ < 3 // 2.95 do not do iso99 variadic macros
#define INFO(dummy,__VA_ARGS__...) do { \
  STDERR_BEGIN(INFO);          \
  /*STDERR_PRINTF("%s:%d: ",__FILE__,__LINE__);*/  \
  STDERR_PRINTF(__VA_ARGS__);  \
  STDERR_END;                  \
} while (0)
#else//__LAND02_CODE__
#define INFO(dummy,...) do {   \
  STDERR_BEGIN(INFO);          \
  /*STDERR_PRINTF("%s:%d: \n",__FILE__,__LINE__);*/  \
  STDERR_PRINTF(__VA_ARGS__);  \
  STDERR_END;                  \
} while (0)
#endif
#else
#if defined __GNUC__ && __GNUC__ < 3 // 2.95 do not do iso99 variadic macros
#define INFO(__VA_ARGS__...) do { \
  STDERR_BEGIN(INFO);          \
  /*STDERR_PRINTF("%s:%d: ",__FILE__,__LINE__);*/  \
  STDERR_PRINTF(__VA_ARGS__);  \
  STDERR_END;                  \
} while (0)
#else
#define INFO(...) do {         \
  STDERR_BEGIN(INFO);          \
/*STDERR_PRINTF("%s:%d: ",__FILE__,__LINE__);*/  \
  STDERR_PRINTF(__VA_ARGS__);  \
  STDERR_END;                  \
} while (0)
#endif
#endif//__LAND02_CODE__

#if defined __GNUC__ && __GNUC__ < 3 // 2.95 do not do iso99 variadic macros
#define ERROR_U_LOC(loc,__VA_ARGS__...) do { \
  STDERR_BEGIN(ERROR);            \
  ucesb_loc_print_lineno(STDERR_FILE,(loc)); \
  STDERR_PUTC(' ');               \
  STDERR_PRINTF(__VA_ARGS__);     \
  STDERR_END;                     \
  throw error();		  \
} while (0)
#else
#define ERROR_U_LOC(loc,...) do { \
  STDERR_BEGIN(ERROR);            \
  ucesb_loc_print_lineno(STDERR_FILE,(loc)); \
  STDERR_PUTC(' ');               \
  STDERR_PRINTF(__VA_ARGS__);     \
  STDERR_END;                     \
  throw error();		  \
} while (0)
#endif

#if defined __GNUC__ && __GNUC__ < 3 // 2.95 do not do iso99 variadic macros
#define ERROR_LOC(loc,__VA_ARGS__...) do { \
  STDERR_BEGIN(ERROR);            \
  (loc).print_lineno(stderr);     \
  STDERR_PUTC(' ');               \
  STDERR_PRINTF(__VA_ARGS__);     \
  STDERR_END;                     \
  throw error();		  \
} while (0)
#else
#define ERROR_LOC(loc,...) do {   \
  STDERR_BEGIN(ERROR);            \
  (loc).print_lineno(stderr);     \
  STDERR_PUTC(' ');               \
  STDERR_PRINTF(__VA_ARGS__);     \
  STDERR_END;                     \
  throw error();		  \
} while (0)
#endif


#endif//__ERROR_HH__
