#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#ifdef NO_DEBUG
#define debug(...)
#else
#define debug(...) printf(__VA_ARGS__)
#endif
