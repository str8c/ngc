#include "../main.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#include <pthread.h>

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static const int visual_attribs[] = {
    GLX_X_RENDERABLE, True,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, DEPTH_SIZE,
    GLX_STENCIL_SIZE, STENCIL_SIZE,
    GLX_DOUBLEBUFFER, True,
    None
};

static const int context_attribs[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 0,
    //GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    None
};

static volatile _Bool redraw, done, preinit_done;

void ngc_redraw(void)
{
    redraw = 1;
}

void ngc_exit(void)
{
    done = 1;
}

void ngc_thread(void (func)(void*), void *args)
{
    pthread_t thread;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1 << 16);
    pthread_create(&thread, &attr, (void*(*)(void*))func, args);
    pthread_attr_destroy(&attr);
}

void ngc_togglekeyboard(_Bool enable)
{
    /* do nothing (hardware keyboard) */
}

static void preinit(void *args)
{
    do_preinit();
    preinit_done = 1;
}

#define TICKS_PER_SEC ((uint64_t)1000 * 1000 * 1000)
#define TICKS_PER_FRAME (TICKS_PER_SEC / GAME_FPS)

static uint64_t get_time(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ((uint64_t)ts.tv_sec * 1000 * 1000 * 1000) + (uint64_t)ts.tv_nsec;
}

int main(int argc, char* argv[])
{
    Display *display;
    Window win;
    GLXContext ctx;
    GLXFBConfig *fbc, fb;
    XVisualInfo *vi;
    XSetWindowAttributes swa;
    glXCreateContextAttribsARBProc glXCreateContextAttribs;
    int glx_major, glx_minor;
    int samp_buf, samples;
    int cw_flags;
    int i;
    int64_t now, then;
    uint8_t frames;

    ngc_thread(preinit, NULL);

    if(!(display = XOpenDisplay(NULL))) {
        debug("XOpenDisplay() failed\n");
        return 0;
    }

    if(!glXQueryVersion(display, &glx_major, &glx_minor)) {
        debug("glXQueryVersion() failed\n");
        goto EXIT_FREE_DISPLAY;
    }

    /* verify version >= 1.3 (note: GLX version, not GL version) */
    if(glx_major < 1 || (glx_major == 1 && glx_minor < 3)) {
        debug("Invalid GLX version (%i.%i)\n", glx_major, glx_minor);
        goto EXIT_FREE_DISPLAY;
    }

    if(!(fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &i))) {
        debug("glXChooseFBConfig failed\n");
        goto EXIT_FREE_DISPLAY;
    }

    debug("%u matching FB configs\n", i);

    //TODO: choose fbc correctly
    fb = NULL;
    while(--i >= 0) {
        if(!(vi = glXGetVisualFromFBConfig(display, fbc[i]))) {
            continue;
        }

        glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
        glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES, &samples);
        debug("visualid=0x%x, SAMPLE_BUFFERS=%u, SAMPLES=%u\n", (int)vi->visualid, samp_buf, samples);
        XFree(vi);
        fb = fbc[i];
    }

    XFree(fbc);
    if(!fb) {
        goto EXIT_FREE_DISPLAY;
    }

    vi = glXGetVisualFromFBConfig(display, fb);
    debug("chosen visualid=0x%x\n", (int)vi->visualid);

    win = RootWindow(display, vi->screen);

    swa.colormap = XCreateColormap(display, win, vi->visual, AllocNone);
    swa.background_pixmap = None;
    swa.border_pixel = 0;
    swa.event_mask = ButtonPressMask | PointerMotionMask | KeyPressMask | StructureNotifyMask;
    cw_flags = (CWBorderPixel | CWColormap | CWEventMask);

    win = XCreateWindow(display, win, 0, 0, WIDTH, HEIGHT, 0, vi->depth, InputOutput, vi->visual, cw_flags, &swa);
    XFree(vi);

    if(!win) {
        debug("XCreateWindow() failed\n");
        goto EXIT_FREE_COLORMAP;
    }

    XStoreName(display, win, NAME);
    XMapWindow(display, win);

    if(!(glXCreateContextAttribs = (void*)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB"))) {
        debug("glXCreateContextAttribsARB() not found\n");
        ctx = glXCreateNewContext(display, fb, GLX_RGBA_TYPE, 0, True);
    } else {
        ctx = glXCreateContextAttribs(display, fb, 0, True, context_attribs);
    }

    if(!ctx) {
        debug("context creation failed\n");
        goto EXIT_FREE_WINDOW;
    }

    if(!glXIsDirect(display, ctx)) {
        debug("Indirect GLX rendering context\n");
    }

    glXMakeCurrent(display, win, ctx);

    while(!preinit_done) {
        //
    }

    if(!do_init(WIDTH, HEIGHT)) {
        goto EXIT_INIT_FAIL;
    }

    frames = GAME_FPS;
    then = get_time() + TICKS_PER_SEC;
    do {
        now = get_time();
        if(((then - now) / frames) < TICKS_PER_FRAME) {
            if(--frames == 0) {
                then += TICKS_PER_SEC;
                frames = GAME_FPS;
            }

            do_frame();
        }

        if(redraw) {
            redraw = 0;
            do_draw();
            glXSwapBuffers(display, win);
        }
    } while(!done);

    do_cleanup();

EXIT_INIT_FAIL:
    glXMakeCurrent(display, 0, 0);
    glXDestroyContext(display, ctx);
EXIT_FREE_WINDOW:
    XDestroyWindow(display, win);
EXIT_FREE_COLORMAP:
    XFreeColormap(display, swa.colormap);
EXIT_FREE_DISPLAY:
    XCloseDisplay(display);
    return 0;
}
