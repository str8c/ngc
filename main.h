#ifndef NGC_MAIN_H
#define NGC_MAIN_H
#include <stdint.h>
#include <stdbool.h>

/* default game config */
#ifndef GAME_FPS
#define GAME_FPS 60
#endif

/* defult GL config */
#ifndef DEPTH_SIZE
#define DEPTH_SIZE 16
#endif

#ifndef STENCIL_SIZE
#define STENCIL_SIZE 0
#endif

/* default window config */
#ifndef WIDTH
#define WIDTH 1024
#endif

#ifndef HEIGHT
#define HEIGHT 768
#endif

#ifndef NAME
#define NAME "Default name"
#endif

/* other default config */
#ifndef NO_DEBUG
#define DEBUG
#endif

/* operating system includes */
#ifdef XLIB
#include "xlib/main.h"
#endif

#ifdef __WIN32__
#include "win32/main.h"
#endif

#ifdef WAYLAND
#include "wayland/main.h"
#endif

#ifdef __ANDROID__
#include "android/main.h"
#endif

/* tool includes */
#ifdef SHADERS
#include "tools/shaders.h"
#endif

/* useful macros */
#define countof(x) (sizeof(x) / sizeof(*x))

/* GLOBAL variables */

/* EVENTS: functions called by the engine, implemented by the game */

/* called (from input thread) on keyboard input
 */
void do_keydown(uint32_t keycode, uint32_t unicode);
void do_keyup(uint32_t keycode, uint32_t unicode);

/* called (from input thread) on mouse (pointer) events
 */
void do_pmove(uint8_t id, uint16_t x, uint16_t y);
void do_pdown(uint8_t id, uint16_t x, uint16_t y);
void do_pup(uint8_t id, uint16_t x, uint16_t y);
void do_pleave(uint8_t id, uint16_t x, uint16_t y);

/* called (from unique thread) from the start
    note: GL is not initialized
 */
void do_preinit(void);

/* called (from main thread) after GL has been loaded and do_preinit has returned
    return
        0 exit
        1 success
 */
bool do_init(uint16_t width, uint16_t height);

/* called (from main thread) on exit
 */
void do_cleanup(void);

/* called (from main thread) GAME_FPS times per second
 */
void do_frame(void);

/* called (from main thread) when rendering needs to be done
 */
void do_draw(void);

/* FUNCTIONS used by game */

/* cause redraw
 */
void ngc_redraw(void);

/* quit
 */
void ngc_exit(void);

/* start a new thread
 */
void ngc_thread(void (func)(void*), void *args);

/* enable/disable keyboard on platforms where the keyboard can be enabled/disabled (ex: android)
 */
void ngc_togglekeyboard(bool enable);

#endif // NGC_MAIN_H
