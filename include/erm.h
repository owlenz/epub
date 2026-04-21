#ifndef ERM_H
#define ERM_H

#ifdef DEBUG
#define ASSERT(expr)                                                           \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #expr,       \
              __FILE__, __LINE__);                                             \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#define DEBUG_PRINT(fmt, ...)                                                  \
  fprintf(stderr, "DEBUG, %s:%d:%s(): " fmt "\n", __FILE__, __LINE__,          \
          __func__, ##__VA_ARGS__)
#else

#define ASSERT(expr)
#define DEBUG_PRINT(fmt, ...)

#endif


#endif
