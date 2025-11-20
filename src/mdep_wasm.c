#include "key.h"
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>

void
mdep_hello(int argc, char **argv)
{
    // Initialize things if needed
}

void
mdep_bye(void)
{
    // Cleanup
}

int
mdep_changedir(char *d)
{
    return chdir(d);
}

char *
mdep_currentdir(char *buff, int leng)
{
    return getcwd(buff, leng);
}

int
mdep_lsdir(char *dir, char *exp, void (*callback)(char *, int))
{
    DIR *d;
    struct dirent *dirEntry;

    d = opendir(dir);
    if (d) {
        while ((dirEntry = readdir(d)) != NULL) {
            // Simple filter, maybe improve later to match 'exp' pattern
            callback(dirEntry->d_name, (dirEntry->d_type == DT_DIR));
        }
        closedir(d);
    }
    return 0;
}

long
mdep_filetime(char *fn)
{
    struct stat s;
    if (stat(fn, &s) == -1)
        return -1;
    return (long)s.st_mtime;
}

int
mdep_fisatty(FILE *f)
{
    return isatty(fileno(f));
}

long
mdep_currtime(void)
{
    time_t t;
    time(&t);
    return (long)t;
}

long
mdep_coreleft(void)
{
    return 1024 * 1024 * 1024; // Fake 1GB free
}

int
mdep_full_or_relative_path(char *path)
{
    if (*path == '/' || *path == '.')
        return 1;
    return 0;
}

int
mdep_makepath(char *dirname, char *filename, char *result, int resultsize)
{
    if (resultsize < (int)(strlen(dirname) + strlen(filename) + 2))
        return 1;
    
    if (strcmp(dirname, ".") == 0) {
        strcpy(result, filename);
        return 0;
    }

    strcpy(result, dirname);
    if (*dirname != '\0' && dirname[strlen(dirname)-1] != '/')
        strcat(result, "/");
    strcat(result, filename);
    return 0;
}

void
mdep_popup(char *s)
{
    fprintf(stderr, "POPUP: %s\n", s);
}

void
mdep_setcursor(int c)
{
    // No-op for now
}

void
mdep_prerc(void)
{
    // No-op
}

void
mdep_abortexit(char *msg)
{
    fprintf(stderr, "ABORT: %s\n", msg);
    exit(1);
}

void
mdep_setinterrupt(SIGFUNCTYPE func)
{
    signal(SIGINT, func);
}

void
mdep_sync(void)
{
    // No-op
}

static long start_time_ms = 0;

long
mdep_milliclock(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    if (start_time_ms == 0)
        start_time_ms = ms;
    return ms - start_time_ms;
}

void
mdep_resetclock(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    start_time_ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

// MIDI Stubs - Enhanced implementations for compilation
// These provide proper structure initialization even though MIDI won't work yet

// Static storage for MIDI device names
static char midi_in_name_0[] = "WASM MIDI In 1";
static char midi_in_name_1[] = "WASM MIDI In 2";
static char midi_out_name_0[] = "WASM MIDI Out 1";
static char midi_out_name_1[] = "WASM MIDI Out 2";

// Number of virtual MIDI devices to report
#define WASM_MIDI_IN_DEVICES 2
#define WASM_MIDI_OUT_DEVICES 2

int
mdep_getnmidi(char *buff, int buffsize, int *port)
{
    // No MIDI input available yet
    // Return 0 to indicate no bytes read
    if (port)
        *port = 0;
    return 0;
}

void
mdep_putnmidi(int n, char *cp, Midiport *pport)
{
    // Stub: Accept MIDI output but don't do anything with it
    // In a real implementation, this would send to Web MIDI API
    // For now, just silently accept the data
    (void)n;
    (void)cp;
    (void)pport;
}

int
mdep_initmidi(Midiport *inputs, Midiport *outputs)
{
    int i;
    
    // Initialize MIDI input ports
    for (i = 0; i < MIDI_IN_DEVICES; i++) {
        if (i < WASM_MIDI_IN_DEVICES) {
            // Set up virtual MIDI input device
            switch (i) {
                case 0:
                    inputs[i].name = midi_in_name_0;
                    break;
                case 1:
                    inputs[i].name = midi_in_name_1;
                    break;
                default:
                    inputs[i].name = NULL;
                    break;
            }
            inputs[i].opened = 0;
            inputs[i].private1 = i;
        } else {
            // No device at this index
            inputs[i].name = NULL;
            inputs[i].opened = 0;
            inputs[i].private1 = -1;
        }
    }
    
    // Initialize MIDI output ports
    for (i = 0; i < MIDI_OUT_DEVICES; i++) {
        if (i < WASM_MIDI_OUT_DEVICES) {
            // Set up virtual MIDI output device
            switch (i) {
                case 0:
                    outputs[i].name = midi_out_name_0;
                    break;
                case 1:
                    outputs[i].name = midi_out_name_1;
                    break;
                default:
                    outputs[i].name = NULL;
                    break;
            }
            outputs[i].opened = 0;
            outputs[i].private1 = i;
        } else {
            // No device at this index
            outputs[i].name = NULL;
            outputs[i].opened = 0;
            outputs[i].private1 = -1;
        }
    }
    
    // TODO: In a real implementation, enumerate Web MIDI API devices here
    // and populate the inputs/outputs arrays with actual device names
    
    return 0; // Success
}

void
mdep_endmidi(void)
{
    // Cleanup MIDI resources
    // TODO: Close any open Web MIDI API connections
}

int
mdep_midi(int openclose, Midiport *p)
{
    // Handle MIDI port open/close operations
    // For now, just pretend they succeed
    
    if (p == NULL)
        return -1;
    
    switch (openclose) {
        case MIDI_OPEN_INPUT:
            // Pretend to open MIDI input
            p->opened = 1;
            return 0;
            
        case MIDI_CLOSE_INPUT:
            // Pretend to close MIDI input
            p->opened = 0;
            return 0;
            
        case MIDI_OPEN_OUTPUT:
            // Pretend to open MIDI output
            p->opened = 1;
            return 0;
            
        case MIDI_CLOSE_OUTPUT:
            // Pretend to close MIDI output
            p->opened = 0;
            return 0;
            
        default:
            // Unknown operation
            return -1;
    }
}

// Generic mdep entry point
Datum
mdep_mdep(int argc)
{
    return numdatum(0);
}

int
mdep_waitfor(int millimsecs)
{
    // Simple sleep for now, but in browser main loop this might block
    // Emscripten can handle sleep if using Asyncify, or we might need to change how the main loop works.
    // For now, return K_TIMEOUT immediately or after sleep.
    if (millimsecs > 0)
        usleep(millimsecs * 1000);
    return K_TIMEOUT;
}

int
mdep_getportdata(PORTHANDLE *port, char *buff, int max, Datum *data)
{
    return -1; // No ports
}

int
mdep_getconsole(void)
{
    return -1; // No console input for now
}

int
mdep_statconsole()
{
    return 0;
}

// Graphics and windowing functions
int
mdep_maxx(void)
{
    return 1024; // Default width
}

int
mdep_maxy(void)
{
    return 768; // Default height
}

int
mdep_fontwidth(void)
{
    return 8; // Default monospace width
}

int
mdep_fontheight(void)
{
    return 16; // Default monospace height
}

void
mdep_line(int x0, int y0, int x1, int y1)
{
    // TODO: Implement with Canvas API via JavaScript
}

void
mdep_string(int x, int y, char *s)
{
    // TODO: Implement with Canvas API via JavaScript
}

void
mdep_color(int c)
{
    // TODO: Set drawing color
}

void
mdep_box(int x0, int y0, int x1, int y1)
{
    // TODO: Draw rectangle outline
}

void
mdep_boxfill(int x0, int y0, int x1, int y1)
{
    // TODO: Draw filled rectangle
}

void
mdep_ellipse(int x0, int y0, int x1, int y1)
{
    // TODO: Draw ellipse outline
}

void
mdep_fillellipse(int x0, int y0, int x1, int y1)
{
    // TODO: Draw filled ellipse
}

void
mdep_fillpolygon(int *x, int *y, int n)
{
    // TODO: Draw filled polygon
}

void
mdep_freebitmap(Pbitmap b)
{
    // TODO: Free bitmap memory
}

int
mdep_startgraphics(int argc, char **argv)
{
    // TODO: Initialize graphics system (Canvas, etc.)
    return 0;
}

void
mdep_startrealtime(void)
{
    // TODO: Start realtime mode
}

void
mdep_startreboot(void)
{
    // TODO: Handle reboot
}

void
mdep_endgraphics(void)
{
    // TODO: Cleanup graphics
}

void
mdep_plotmode(int mode)
{
    // TODO: Set plot mode (XOR, etc.)
}

int
mdep_screensize(int *x0, int *y0, int *x1, int *y1)
{
    *x0 = 0;
    *y0 = 0;
    *x1 = mdep_maxx();
    *y1 = mdep_maxy();
    return 0;
}

int
mdep_screenresize(int x0, int y0, int x1, int y1)
{
    // TODO: Resize screen/canvas
    return 0;
}

// Font functions
char *
mdep_fontinit(char *fnt)
{
    // Return default font name
    return "monospace";
}

// Mouse functions
int
mdep_mouse(int *ax, int *ay, int *am)
{
    // TODO: Get mouse position and buttons
    *ax = 0;
    *ay = 0;
    *am = 0;
    return 0;
}

int
mdep_mousewarp(int x, int y)
{
    // TODO: Move mouse cursor
    return 0;
}

// Color functions
void
mdep_colormix(int n, int r, int g, int b)
{
    // TODO: Set color palette entry
}

void
mdep_initcolors(void)
{
    // TODO: Initialize color palette
}

// Bitmap functions (Pbitmap is defined in grid.h)
Pbitmap
mdep_allocbitmap(int xsize, int ysize)
{
    Pbitmap pb = (Pbitmap)malloc(sizeof(struct Pbitmap_struct));
    if (pb) {
        pb->xsize = xsize;
        pb->ysize = ysize;
        pb->origx = xsize;
        pb->origy = ysize;
        pb->ptr = NULL; // TODO: Allocate actual bitmap data
    }
    return pb;
}

Pbitmap
mdep_reallocbitmap(int xsize, int ysize, Pbitmap pb)
{
    if (pb) {
        pb->xsize = xsize;
        pb->ysize = ysize;
        // TODO: Reallocate bitmap data
    }
    return pb;
}

void
mdep_movebitmap(int fromx0, int fromy0, int width, int height, int tox0, int toy0)
{
    // TODO: Move bitmap region
}

void
mdep_pullbitmap(int x0, int y0, Pbitmap pb)
{
    // TODO: Copy from screen to bitmap
}

void
mdep_putbitmap(int x0, int y0, Pbitmap pb)
{
    // TODO: Copy from bitmap to screen
}

void
mdep_destroywindow(void)
{
    // TODO: Destroy window
}

// File/path functions
char *
mdep_keypath(void)
{
    return "/keykit/lib";
}

char *
mdep_musicpath(void)
{
    return "/keykit/music";
}

void
mdep_postrc(void)
{
    // Post-initialization
}

int
mdep_shellexec(char *s)
{
    // Can't execute shell commands in browser
    return -1;
}

void
mdep_ignoreinterrupt(void)
{
    signal(SIGINT, SIG_IGN);
}

char *
mdep_browse(char *desc, char *types, int mustexist)
{
    // TODO: Implement file browser dialog
    return NULL;
}

// Port functions
PORTHANDLE *
mdep_openport(char *name, char *mode, char *type)
{
    return NULL;
}

int
mdep_putportdata(PORTHANDLE m, char *buff, int size)
{
    return -1;
}

int
mdep_closeport(PORTHANDLE m)
{
    return -1;
}

Datum
mdep_ctlport(PORTHANDLE m, char *cmd, char *arg)
{
    Datum d;
    d.type = D_NUM;
    d.u.val = -1;
    return d;
}

int
mdep_help(char *fname, char *keyword)
{
    return -1;
}

char *
mdep_localaddresses(Datum d)
{
    return "127.0.0.1";
}
