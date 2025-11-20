/*
 * KeyKit Graphics Implementation for WebAssembly
 *
 * This file shows how to replace the graphics stubs in mdep_wasm.c
 * with actual Canvas drawing implementations.
 *
 * Copy these implementations into your mdep_wasm.c file.
 */

#include "key.h"
#include <emscripten.h>
#include <math.h>

// Global state for graphics
static int current_color = 0;
static int line_width = 1;

// Color palette (can be customized)
static const char *color_palette[] = {
    "#000000", // 0 - Black
    "#FFFFFF", // 1 - White
    "#FF0000", // 2 - Red
    "#00FF00", // 3 - Green
    "#0000FF", // 4 - Blue
    "#FFFF00", // 5 - Yellow
    "#FF00FF", // 6 - Magenta
    "#00FFFF", // 7 - Cyan
    "#808080", // 8 - Gray
    "#C0C0C0", // 9 - Light Gray
    "#800000", // 10 - Maroon
    "#008000", // 11 - Dark Green
    "#000080", // 12 - Navy
    "#808000", // 13 - Olive
    "#800080", // 14 - Purple
    "#008080"  // 15 - Teal
};

#define NUM_COLORS (sizeof(color_palette) / sizeof(color_palette[0]))

// Helper function to get color string
static const char *get_color_string(int color_index) {
  if (color_index < 0 || color_index >= NUM_COLORS) {
    return color_palette[0]; // Default to black
  }
  return color_palette[color_index];
}

// ============================================================================
// GRAPHICS FUNCTIONS - Replace these in mdep_wasm.c
// ============================================================================

void mdep_line(int x0, int y0, int x1, int y1) {
  EM_ASM(
      {
        var canvas = document.getElementById('keykit-canvas');
        if (!canvas)
          return;
        var ctx = canvas.getContext('2d');
        ctx.beginPath();
        ctx.moveTo($0, $1);
        ctx.lineTo($2, $3);
        ctx.stroke();
      },
      x0, y0, x1, y1);
}

void mdep_box(int x0, int y0, int x1, int y1) {
  int width = x1 - x0;
  int height = y1 - y0;

  EM_ASM(
      {
        var canvas = document.getElementById('keykit-canvas');
        if (!canvas)
          return;
        var ctx = canvas.getContext('2d');
        ctx.strokeRect($0, $1, $2, $3);
      },
      x0, y0, width, height);
}

void mdep_boxfill(int x0, int y0, int x1, int y1) {
  int width = x1 - x0;
  int height = y1 - y0;

  EM_ASM(
      {
        var canvas = document.getElementById('keykit-canvas');
        if (!canvas)
          return;
        var ctx = canvas.getContext('2d');
        ctx.fillRect($0, $1, $2, $3);
      },
      x0, y0, width, height);
}

void mdep_string(int x, int y, char *s) {
  EM_ASM(
      {
        var canvas = document.getElementById('keykit-canvas');
        if (!canvas)
          return;
        var ctx = canvas.getContext('2d');
        ctx.fillText(UTF8ToString($2), $0, $1);
      },
      x, y, s);
}

void mdep_color(int c) {
  current_color = c;
  const char *color_str = get_color_string(c);

  EM_ASM(
      {
        var canvas = document.getElementById('keykit-canvas');
        if (!canvas)
          return;
        var ctx = canvas.getContext('2d');
        var color = UTF8ToString($0);
        ctx.strokeStyle = color;
        ctx.fillStyle = color;
      },
      color_str);
}

void mdep_colormix(int n, int r, int g, int b) {
  // Create RGB color string
  char color_str[32];
  sprintf(color_str, "rgb(%d,%d,%d)", r, g, b);

  EM_ASM(
      {
        var canvas = document.getElementById('keykit-canvas');
        if (!canvas)
          return;
        var ctx = canvas.getContext('2d');
        var color = UTF8ToString($0);
        ctx.strokeStyle = color;
        ctx.fillStyle = color;
      },
      color_str);
}

void mdep_ellipse(int x0, int y0, int x1, int y1) {
  EM_ASM(
      {
        var canvas = document.getElementById('keykit-canvas');
        if (!canvas)
          return;
        var ctx = canvas.getContext('2d');

        var centerX = ($0 + $2) / 2;
        var centerY = ($1 + $3) / 2;
        var radiusX = Math.abs($2 - $0) / 2;
        var radiusY = Math.abs($3 - $1) / 2;

        ctx.beginPath();
        ctx.ellipse(centerX, centerY, radiusX, radiusY, 0, 0, 2 * Math.PI);
        ctx.stroke();
      },
      x0, y0, x1, y1);
}

void mdep_fillellipse(int x0, int y0, int x1, int y1) {
  EM_ASM(
      {
        var canvas = document.getElementById('keykit-canvas');
        if (!canvas)
          return;
        var ctx = canvas.getContext('2d');

        var centerX = ($0 + $2) / 2;
        var centerY = ($1 + $3) / 2;
        var radiusX = Math.abs($2 - $0) / 2;
        var radiusY = Math.abs($3 - $1) / 2;

        ctx.beginPath();
        ctx.ellipse(centerX, centerY, radiusX, radiusY, 0, 0, 2 * Math.PI);
        ctx.fill();
      },
      x0, y0, x1, y1);
}

void mdep_fillpolygon(int *x, int *y, int n) {
  if (n < 3)
    return; // Need at least 3 points

  EM_ASM(
      {
        var canvas = document.getElementById('keykit-canvas');
        if (!canvas)
          return;
        var ctx = canvas.getContext('2d');
        var numPoints = $2;

        ctx.beginPath();

        // Get first point
        var firstX = getValue($0, 'i32');
        var firstY = getValue($1, 'i32');
        ctx.moveTo(firstX, firstY);

        // Draw to remaining points
        for (var i = 1; i < numPoints; i++) {
          var px = getValue($0 + i * 4, 'i32');
          var py = getValue($1 + i * 4, 'i32');
          ctx.lineTo(px, py);
        }

        ctx.closePath();
        ctx.fill();
      },
      x, y, n);
}

void mdep_plotmode(int mode) {
  // mode: P_CLEAR=0, P_STORE=1, P_XOR=2
  const char *composite_op;

  switch (mode) {
  case 0: // P_CLEAR
    composite_op = "destination-out";
    break;
  case 2: // P_XOR
    composite_op = "xor";
    break;
  case 1: // P_STORE
  default:
    composite_op = "source-over";
    break;
  }

  EM_ASM(
      {
        var canvas = document.getElementById('keykit-canvas');
        if (!canvas)
          return;
        var ctx = canvas.getContext('2d');
        ctx.globalCompositeOperation = UTF8ToString($0);
      },
      composite_op);
}

void mdep_sync(void) {
  // Force canvas to update (usually automatic)
  EM_ASM({
    var canvas = document.getElementById('keykit-canvas');
    if (!canvas)
      return;
    var ctx = canvas.getContext('2d');
    // Canvas updates are automatic, but we can force a repaint
    canvas.style.display = 'none';
    canvas.offsetHeight; // Trigger reflow
    canvas.style.display = 'block';
  });
}

int mdep_startgraphics(int argc, char **argv) {
  // Initialize canvas
  EM_ASM({
    var canvas = document.getElementById('keykit-canvas');
    if (!canvas) {
      console.error('Canvas element not found!');
      return;
    }

    var ctx = canvas.getContext('2d');

    // Set default drawing properties
    ctx.strokeStyle = '#000000';
    ctx.fillStyle = '#000000';
    ctx.lineWidth = 1;
    ctx.font = '12px monospace';
    ctx.textBaseline = 'top';

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    console.log('Graphics initialized');
  });

  return 0;
}

void mdep_endgraphics(void) {
  // Cleanup if needed
  EM_ASM({ console.log('Graphics cleanup'); });
}

// Font functions
char *mdep_fontinit(char *fnt) {
  if (fnt) {
    EM_ASM(
        {
          var canvas = document.getElementById('keykit-canvas');
          if (!canvas)
            return;
          var ctx = canvas.getContext('2d');
          ctx.font = UTF8ToString($0);
        },
        fnt);
  }
  return fnt ? fnt : "12px monospace";
}

int mdep_fontwidth(void) {
  return EM_ASM_INT({
    var canvas = document.getElementById('keykit-canvas');
    if (!canvas)
      return 8;
    var ctx = canvas.getContext('2d');
    // Measure a typical character
    var metrics = ctx.measureText('M');
    return Math.ceil(metrics.width);
  });
}

int mdep_fontheight(void) {
  return EM_ASM_INT({
    var canvas = document.getElementById('keykit-canvas');
    if (!canvas)
      return 16;
    var ctx = canvas.getContext('2d');
    // Parse font size from font string
    var font = ctx.font;
    var size = parseInt(font);
    return size || 16;
  });
}

// Bitmap functions (simplified - you may need more complex implementation)
Pbitmap mdep_allocbitmap(int xsize, int ysize) {
  Pbitmap pb = (Pbitmap)malloc(sizeof(struct Pbitmap));
  if (pb) {
    pb->xsize = xsize;
    pb->ysize = ysize;
    pb->origx = xsize;
    pb->origy = ysize;

    // Allocate ImageData in JavaScript
    int id = EM_ASM_INT(
        {
          var canvas = document.createElement('canvas');
          canvas.width = $0;
          canvas.height = $1;
          var ctx = canvas.getContext('2d');
          var imageData = ctx.createImageData($0, $1);

          // Store in global array (you'll need to manage this)
          if (!Module.bitmaps)
            Module.bitmaps = [];
          Module.bitmaps.push(imageData);
          return Module.bitmaps.length - 1;
        },
        xsize, ysize);

    pb->ptr = (unsigned char *)(intptr_t)id; // Store ID as pointer
  }
  return pb;
}

void mdep_freebitmap(Pbitmap pb) {
  if (pb) {
    int id = (int)(intptr_t)pb->ptr;
    EM_ASM(
        {
          if (Module.bitmaps && Module.bitmaps[$0]) {
            Module.bitmaps[$0] = null;
          }
        },
        id);
    free(pb);
  }
}

// ============================================================================
// USAGE NOTES
// ============================================================================

/*
 * To use these implementations:
 *
 * 1. Copy these functions into your mdep_wasm.c file
 * 2. Make sure to include <emscripten.h> at the top
 * 3. Update your HTML to include a canvas with id="keykit-canvas"
 * 4. Compile with: emcc ... -s
 * EXPORTED_RUNTIME_METHODS='["ccall","cwrap","getValue","UTF8ToString"]'
 * 5. Test with simple drawing operations first
 *
 * HTML Example:
 * <canvas id="keykit-canvas" width="1024" height="768"></canvas>
 *
 * The canvas will be automatically initialized when mdep_startgraphics() is
 * called.
 */
