/* tools code */
#ifdef SHADERS
#include "tools/shaders.c"
#endif

/* operating system code */
#ifdef XLIB
#include "xlib/main.c"
#endif

#ifdef __WIN32__
#include "win32/main.c"
#endif

#ifdef WAYLAND
#include "wayland/main.c"
#endif

#ifdef __ANDROID__
#include "android/main.c"
#endif
