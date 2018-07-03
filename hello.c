#include <EGL/egl.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

// X11 related local variables
static Display *x_display = NULL;

///
//  createNativeWindow()
//
//      This function initialized the native X11 display and window for EGL
//
EGLNativeWindowType createNativeWindow(const char *title) {
  Window root;
  XSetWindowAttributes swa;
  XSetWindowAttributes xattr;
  Atom wm_state;
  XWMHints hints;
  XEvent xev;
  EGLConfig ecfg;
  EGLint num_config;
  Window win;

  /*
   * X11 native display initialization
   */

  x_display = XOpenDisplay(NULL);
  if (x_display == NULL) {
    return EGL_FALSE;
  }

  root = DefaultRootWindow(x_display);

  swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;
  win = XCreateWindow(x_display, root, 0, 0, 600, 600, 0, CopyFromParent,
                      InputOutput, CopyFromParent, CWEventMask, &swa);

  xattr.override_redirect = FALSE;
  XChangeWindowAttributes(x_display, win, CWOverrideRedirect, &xattr);

  hints.input = TRUE;
  hints.flags = InputHint;
  XSetWMHints(x_display, win, &hints);

  // make the window visible on the screen
  XMapWindow(x_display, win);
  XStoreName(x_display, win, title);

  // get identifiers for the provided atom name strings
  wm_state = XInternAtom(x_display, "_NET_WM_STATE", FALSE);

  memset(&xev, 0, sizeof(xev));
  xev.type = ClientMessage;
  xev.xclient.window = win;
  xev.xclient.message_type = wm_state;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = 1;
  xev.xclient.data.l[1] = FALSE;
  XSendEvent(x_display, DefaultRootWindow(x_display), FALSE,
             SubstructureNotifyMask, &xev);

  return (EGLNativeWindowType)win;
}

static EGLint const attribute_list[] = {EGL_RED_SIZE,  1, EGL_GREEN_SIZE, 1,
                                        EGL_BLUE_SIZE, 1, EGL_NONE};

static EGLint const contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE,
                                        EGL_NONE};

int main(int argc, char **argv) {
  EGLDisplay display;
  EGLConfig config;
  EGLContext context;
  EGLSurface surface;
  EGLNativeWindowType native_window;
  EGLint num_config;
  /* get an EGL display connection */
  display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  /* initialize the EGL display connection */
  eglInitialize(display, NULL, NULL);
  /* get an appropriate EGL frame buffer configuration */
  eglChooseConfig(display, attribute_list, &config, 1, &num_config);
  /* create an EGL rendering context */
  context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
  /* create a native window */
  native_window = createNativeWindow("abcd");
  /* create an EGL window surface */
  surface = eglCreateWindowSurface(display, config, native_window, NULL);
  /* connect the context to the surface */
  eglMakeCurrent(display, surface, surface, context);
  /* clear the color buffer */
  glClearColor(1.0, 1.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glFlush();
  eglSwapBuffers(display, surface);
  return EXIT_SUCCESS;
}
