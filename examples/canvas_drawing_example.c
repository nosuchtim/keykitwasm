#include <emscripten.h>
#include <math.h>
#include <stdio.h>

// Method 1: Using EM_ASM - Inline JavaScript
void draw_line_inline(int x0, int y0, int x1, int y1) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        ctx.beginPath();
        ctx.moveTo($0, $1);
        ctx.lineTo($2, $3);
        ctx.stroke();
      },
      x0, y0, x1, y1);
}

// Method 2: Using EM_ASM_INT - Return values from JavaScript
int get_canvas_width() {
  return EM_ASM_INT({
    var canvas = document.getElementById('canvas');
    return canvas.width;
  });
}

// Method 3: Using EM_ASM_DOUBLE - For floating point
double get_canvas_aspect_ratio() {
  return EM_ASM_DOUBLE({
    var canvas = document.getElementById('canvas');
    return canvas.width / canvas.height;
  });
}

// Method 4: Drawing text
void draw_text(int x, int y, const char *text) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        ctx.fillText(UTF8ToString($2), $0, $1);
      },
      x, y, text);
}

// Method 5: Drawing a filled rectangle
void draw_filled_rect(int x, int y, int width, int height, const char *color) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        ctx.fillStyle = UTF8ToString($4);
        ctx.fillRect($0, $1, $2, $3);
      },
      x, y, width, height, color);
}

// Method 6: Drawing a circle
void draw_circle(int x, int y, int radius, const char *color) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        ctx.fillStyle = UTF8ToString($3);
        ctx.beginPath();
        ctx.arc($0, $1, $2, 0, 2 * Math.PI);
        ctx.fill();
      },
      x, y, radius, color);
}

// Method 7: Set drawing color
void set_color(const char *color) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        ctx.strokeStyle = UTF8ToString($0);
        ctx.fillStyle = UTF8ToString($0);
      },
      color);
}

// Method 8: Clear canvas
void clear_canvas() {
  EM_ASM({
    var canvas = document.getElementById('canvas');
    var ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);
  });
}

// Method 9: Set line width
void set_line_width(int width) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        ctx.lineWidth = $0;
      },
      width);
}

// Method 10: Draw polygon
void draw_polygon(int *x_points, int *y_points, int num_points,
                  const char *color) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        var numPoints = $2;

        ctx.fillStyle = UTF8ToString($3);
        ctx.beginPath();

        // Get first point
        var x = getValue($0, 'i32');
        var y = getValue($1, 'i32');
        ctx.moveTo(x, y);

        // Draw to remaining points
        for (var i = 1; i < numPoints; i++) {
          x = getValue($0 + i * 4, 'i32');
          y = getValue($1 + i * 4, 'i32');
          ctx.lineTo(x, y);
        }

        ctx.closePath();
        ctx.fill();
      },
      x_points, y_points, num_points, color);
}

// Method 11: Set font
void set_font(const char *font) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        ctx.font = UTF8ToString($0);
      },
      font);
}

// Method 12: Set text alignment
void set_text_align(const char *align) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        ctx.textAlign = UTF8ToString($0);
      },
      align);
}

// Method 13: Set text baseline
void set_text_baseline(const char *baseline) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        ctx.textBaseline = UTF8ToString($0);
      },
      baseline);
}

// Method 14: Draw outlined text
void draw_text_outlined(int x, int y, const char *text, const char *fill_color,
                        const char *stroke_color, int stroke_width) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        var text = UTF8ToString($2);

        // Draw outline
        ctx.strokeStyle = UTF8ToString($4);
        ctx.lineWidth = $5;
        ctx.strokeText(text, $0, $1);

        // Draw fill
        ctx.fillStyle = UTF8ToString($3);
        ctx.fillText(text, $0, $1);
      },
      x, y, text, fill_color, stroke_color, stroke_width);
}

// Method 15: Measure text width
int measure_text_width(const char *text) {
  return EM_ASM_INT(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        var metrics = ctx.measureText(UTF8ToString($0));
        return Math.ceil(metrics.width);
      },
      text);
}

// Method 16: Draw multi-line text
void draw_multiline_text(int x, int y, const char *text, int line_height) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        var text = UTF8ToString($2);
        var lines = text.split('\\n');
        var lineHeight = $3;

        for (var i = 0; i < lines.length; i++) {
          ctx.fillText(lines[i], $0, $1 + i * lineHeight);
        }
      },
      x, y, text, line_height);
}

// Method 17: Draw text with background
void draw_text_with_background(int x, int y, const char *text,
                               const char *text_color, const char *bg_color,
                               int padding) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        var text = UTF8ToString($2);

        // Measure text
        var metrics = ctx.measureText(text);
        var width = metrics.width;
        var height = parseInt(ctx.font);

        // Draw background
        ctx.fillStyle = UTF8ToString($4);
        ctx.fillRect($0 - $5, $1 - height - $5, width + $5 * 2,
                     height + $5 * 2);

        // Draw text
        ctx.fillStyle = UTF8ToString($3);
        ctx.fillText(text, $0, $1);
      },

      x, y, text, text_color, bg_color, padding);
}

// ========== MOUSE EVENT HANDLING ==========

// Global variables to track mouse state
static int mouse_x = 0;
static int mouse_y = 0;
static int mouse_down = 0;
static int drag_start_x = 0;
static int drag_start_y = 0;

// Mouse event callback - called from JavaScript
void on_mouse_event(int event_type, int x, int y) {
  mouse_x = x;
  mouse_y = y;

  // Clear a region for event display
  draw_filled_rect(10, 700, 400, 90, "rgba(0, 0, 0, 0.8)");

  // Set text style
  set_font("14px monospace");
  set_color("lime");

  char msg[256];

  if (event_type == 0) { // Mouse down
    mouse_down = 1;
    drag_start_x = x;
    drag_start_y = y;
    sprintf(msg, "MOUSE DOWN at (%d, %d)", x, y);
    draw_text(20, 720, msg);

    // Draw a circle at click position
    draw_circle(x, y, 5, "yellow");

  } else if (event_type == 1) { // Mouse move/drag
    if (mouse_down) {
      sprintf(msg, "DRAGGING from (%d, %d) to (%d, %d)", drag_start_x,
              drag_start_y, x, y);
      draw_text(20, 720, msg);

      // Draw line from drag start to current position
      set_color("cyan");
      set_line_width(2);
      draw_line_inline(drag_start_x, drag_start_y, x, y);

      // Draw current position
      draw_circle(x, y, 3, "cyan");
    } else {
      sprintf(msg, "MOUSE MOVE at (%d, %d)", x, y);
      draw_text(20, 720, msg);
    }

  } else if (event_type == 2) { // Mouse up
    if (mouse_down) {
      sprintf(msg, "MOUSE UP at (%d, %d)", x, y);
      draw_text(20, 720, msg);

      sprintf(msg, "Drag distance: %d pixels",
              (int)sqrt((x - drag_start_x) * (x - drag_start_x) +
                        (y - drag_start_y) * (y - drag_start_y)));
      draw_text(20, 740, msg);

      // Draw final circle
      draw_circle(x, y, 5, "red");
    }
    mouse_down = 0;
  }

  // Always show current coordinates
  set_color("white");
  sprintf(msg, "Current position: (%d, %d)", x, y);
  draw_text(20, 760, msg);

  sprintf(msg, "Mouse state: %s", mouse_down ? "DOWN" : "UP");
  draw_text(20, 780, msg);
}

// Setup mouse event listeners
void setup_mouse_events() {
  EM_ASM({
    var canvas = document.getElementById('canvas');
    if (!canvas) {
      console.error('Canvas not found!');
      return;
    }

    // Mouse down event
    canvas.addEventListener(
        'mousedown', function(e) {
          var rect = canvas.getBoundingClientRect();
          var x = Math.floor(e.clientX - rect.left);
          var y = Math.floor(e.clientY - rect.top);
          Module.ccall('on_mouse_event', null, [ 'number', 'number', 'number' ],
                       [ 0, x, y ]);
        });

    // Mouse move event
    canvas.addEventListener(
        'mousemove', function(e) {
          var rect = canvas.getBoundingClientRect();
          var x = Math.floor(e.clientX - rect.left);
          var y = Math.floor(e.clientY - rect.top);
          Module.ccall('on_mouse_event', null, [ 'number', 'number', 'number' ],
                       [ 1, x, y ]);
        });

    // Mouse up event
    canvas.addEventListener(
        'mouseup', function(e) {
          var rect = canvas.getBoundingClientRect();
          var x = Math.floor(e.clientX - rect.left);
          var y = Math.floor(e.clientY - rect.top);
          Module.ccall('on_mouse_event', null, [ 'number', 'number', 'number' ],
                       [ 2, x, y ]);
        });

    // Prevent context menu on right click
    canvas.addEventListener('contextmenu', function(e) { e.preventDefault(); });

    console.log('Mouse event listeners set up successfully');
  });
}

// Example main function demonstrating all methods
int main() {
  printf("Drawing on canvas...\n");

  // Clear canvas
  clear_canvas();

  // Setup mouse event handling
  setup_mouse_events();
  printf("Mouse events enabled - try clicking and dragging on the canvas!\n");

  // Draw instructions
  set_font("bold 20px Arial");
  set_color("darkblue");
  draw_text(420, 30, "MOUSE INTERACTION DEMO");

  set_font("16px Arial");
  set_color("black");
  draw_text(420, 60, "Try these interactions:");
  draw_text(420, 85, "• Click anywhere to mark a point");
  draw_text(420, 110, "• Click and drag to draw lines");
  draw_text(420, 135, "• Move mouse to see coordinates");
  draw_text(420, 160, "• Watch the event log at the bottom");

  // Set color and line width
  set_color("blue");
  set_line_width(3);

  // Draw some lines
  draw_line_inline(10, 10, 200, 10);
  draw_line_inline(10, 10, 10, 200);

  // Draw rectangles
  draw_filled_rect(50, 50, 100, 80, "red");
  draw_filled_rect(200, 50, 100, 80, "green");

  // Draw circles
  draw_circle(100, 200, 30, "purple");
  draw_circle(250, 200, 40, "orange");

  // ========== TEXT DRAWING EXAMPLES ==========

  // Basic text
  set_color("white");
  set_font("16px Arial");
  draw_text(10, 10, "Basic text (16px Arial)");

  // Large bold text
  set_font("bold 24px Arial");
  set_color("darkblue");
  draw_text(50, 320, "Large Bold Text");

  // Different fonts
  set_font("italic 18px Georgia");
  set_color("darkgreen");
  draw_text(50, 360, "Italic Georgia font");

  // Monospace font
  set_font("16px 'Courier New', monospace");
  set_color("purple");
  draw_text(50, 390, "Monospace: Code-like text");

  // Text alignment examples
  set_font("16px Arial");
  set_color("black");

  // Left aligned (default)
  set_text_align("left");
  draw_text(400, 280, "Left aligned");

  // Center aligned
  set_text_align("center");
  draw_text(500, 310, "Center aligned");

  // Right aligned
  set_text_align("right");
  draw_text(600, 340, "Right aligned");

  // Reset to left
  set_text_align("left");

  // Outlined text
  set_font("bold 28px Arial");
  draw_text_outlined(400, 390, "Outlined Text!", "yellow", "black", 3);

  // Text with background
  set_font("18px Arial");
  draw_text_with_background(50, 440, "Text with background", "white", "navy",
                            5);

  // Multi-line text
  set_font("14px Arial");
  set_color("black");
  draw_multiline_text(
      50, 480,
      "Line 1: Multi-line text\nLine 2: Second line\nLine 3: Third line", 20);

  // Measure text
  const char *test_text = "Measured text";
  set_font("16px Arial");
  int text_width = measure_text_width(test_text);
  printf("Width of '%s': %d pixels\n", test_text, text_width);

  set_color("red");
  draw_text(400, 480, test_text);

  // Draw a box around the measured text
  set_color("red");
  set_line_width(1);
  draw_line_inline(400, 465, 400 + text_width, 465);
  draw_line_inline(400, 485, 400 + text_width, 485);
  draw_line_inline(400, 465, 400, 485);
  draw_line_inline(400 + text_width, 465, 400 + text_width, 485);

  // Draw a polygon (triangle)
  int x_points[] = {700, 750, 650};
  int y_points[] = {450, 550, 550};
  draw_polygon(x_points, y_points, 3, "cyan");

  // Label the triangle
  set_font("12px Arial");
  set_color("black");
  set_text_align("center");
  draw_text(700, 560, "Triangle");
  set_text_align("left");

  // Get canvas info
  int width = get_canvas_width();
  printf("Canvas width: %d\n", width);

  double aspect = get_canvas_aspect_ratio();
  printf("Canvas aspect ratio: %f\n", aspect);

  // Final message
  set_font("bold 20px Arial");
  set_color("darkred");
  set_text_align("center");
  draw_text(400, 580, "Text Drawing Examples Complete!");

  printf("\nCheck the canvas for all text examples!\n");

  return 0;
}
