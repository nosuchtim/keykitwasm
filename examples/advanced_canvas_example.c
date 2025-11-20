// Method 2: Using JavaScript Library Functions (More Efficient)
// This approach pre-defines JavaScript functions and calls them from C

#include <emscripten.h>
#include <math.h>
#include <stdio.h>

// Declare external JavaScript functions
// These will be defined in a separate .js file or in --js-library

// External declarations for JavaScript functions
extern void js_clear_canvas();
extern void js_draw_line(int x0, int y0, int x1, int y1);
extern void js_draw_rect(int x, int y, int w, int h);
extern void js_fill_rect(int x, int y, int w, int h);
extern void js_draw_circle(int x, int y, int radius);
extern void js_fill_circle(int x, int y, int radius);
extern void js_draw_text(int x, int y, const char *text);
extern void js_set_color(const char *color);
extern void js_set_line_width(int width);
extern void js_set_font(const char *font);
extern int js_get_canvas_width();
extern int js_get_canvas_height();

// Higher-level drawing functions using the JS library
void draw_grid(int spacing, const char *color) {
  int width = js_get_canvas_width();
  int height = js_get_canvas_height();

  js_set_color(color);
  js_set_line_width(1);

  // Vertical lines
  for (int x = 0; x < width; x += spacing) {
    js_draw_line(x, 0, x, height);
  }

  // Horizontal lines
  for (int y = 0; y < height; y += spacing) {
    js_draw_line(0, y, width, y);
  }
}

void draw_coordinate_axes(int origin_x, int origin_y) {
  int width = js_get_canvas_width();
  int height = js_get_canvas_height();

  js_set_color("black");
  js_set_line_width(2);

  // X axis
  js_draw_line(0, origin_y, width, origin_y);

  // Y axis
  js_draw_line(origin_x, 0, origin_x, height);

  // Labels
  js_set_font("14px Arial");
  js_draw_text(width - 20, origin_y - 10, "X");
  js_draw_text(origin_x + 10, 20, "Y");
}

void draw_function_graph(int origin_x, int origin_y, int scale) {
  // Draw a sine wave
  js_set_color("blue");
  js_set_line_width(2);

  int width = js_get_canvas_width();

  for (int x = 0; x < width - 1; x++) {
    double angle1 = (x - origin_x) / (double)scale;
    double angle2 = (x + 1 - origin_x) / (double)scale;

    int y1 = origin_y - (int)(50 * sin(angle1));
    int y2 = origin_y - (int)(50 * sin(angle2));

    js_draw_line(x, y1, x + 1, y2);
  }
}

void draw_bar_chart(int *values, int count, int x_start, int y_base,
                    int bar_width) {
  const char *colors[] = {"red", "green", "blue", "orange", "purple", "cyan"};
  int num_colors = 6;

  for (int i = 0; i < count; i++) {
    int x = x_start + i * (bar_width + 10);
    int height = values[i];
    int y = y_base - height;

    js_set_color(colors[i % num_colors]);
    js_fill_rect(x, y, bar_width, height);

    // Draw value label
    js_set_color("black");
    js_set_font("12px Arial");
    char label[32];
    sprintf(label, "%d", values[i]);
    js_draw_text(x + bar_width / 2 - 10, y - 5, label);
  }
}

void draw_pie_chart(int *values, int count, int center_x, int center_y,
                    int radius) {
  // Calculate total
  int total = 0;
  for (int i = 0; i < count; i++) {
    total += values[i];
  }

  const char *colors[] = {"#FF6384", "#36A2EB", "#FFCE56",
                          "#4BC0C0", "#9966FF", "#FF9F40"};

  double start_angle = 0;

  for (int i = 0; i < count; i++) {
    double slice_angle = (values[i] / (double)total) * 2 * 3.14159265359;

    // Draw pie slice using EM_ASM since we need more complex path operations
    EM_ASM(
        {
          var canvas = document.getElementById('canvas');
          var ctx = canvas.getContext('2d');

          ctx.fillStyle = UTF8ToString($5);
          ctx.beginPath();
          ctx.moveTo($0, $1);
          ctx.arc($0, $1, $2, $3, $3 + $4);
          ctx.closePath();
          ctx.fill();

          // Draw outline
          ctx.strokeStyle = 'white';
          ctx.lineWidth = 2;
          ctx.stroke();
        },
        center_x, center_y, radius, start_angle, slice_angle, colors[i % 6]);

    start_angle += slice_angle;
  }
}

// Animation example
void animate_bouncing_ball() {
  static int x = 50;
  static int y = 50;
  static int dx = 3;
  static int dy = 2;
  static int radius = 20;

  int width = js_get_canvas_width();
  int height = js_get_canvas_height();

  // Clear canvas
  js_clear_canvas();

  // Draw ball
  js_set_color("red");
  js_fill_circle(x, y, radius);

  // Update position
  x += dx;
  y += dy;

  // Bounce off walls
  if (x + radius > width || x - radius < 0) {
    dx = -dx;
  }
  if (y + radius > height || y - radius < 0) {
    dy = -dy;
  }
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

  // Clear event display area
  js_set_color("rgba(0, 0, 0, 0.9)");
  js_fill_rect(10, 750, 500, 40);

  // Set text style
  js_set_font("14px monospace");
  js_set_color("lime");

  char msg[256];

  if (event_type == 0) { // Mouse down
    mouse_down = 1;
    drag_start_x = x;
    drag_start_y = y;
    sprintf(msg, "MOUSE DOWN at (%d, %d) - Drag to draw!", x, y);
    js_draw_text(20, 770, msg);

    // Draw a marker at click position
    js_set_color("yellow");
    js_fill_circle(x, y, 5);

  } else if (event_type == 1) { // Mouse move/drag
    if (mouse_down) {
      sprintf(msg, "DRAGGING from (%d, %d) to (%d, %d)", drag_start_x,
              drag_start_y, x, y);
      js_draw_text(20, 770, msg);

      // Draw line from drag start to current position
      js_set_color("cyan");
      js_set_line_width(3);
      js_draw_line(drag_start_x, drag_start_y, x, y);

      // Draw current position marker
      js_fill_circle(x, y, 4);

      // Update drag start for continuous drawing
      drag_start_x = x;
      drag_start_y = y;
    } else {
      sprintf(msg, "Position: (%d, %d) - Click and drag to draw", x, y);
      js_draw_text(20, 770, msg);
    }

  } else if (event_type == 2) { // Mouse up
    if (mouse_down) {
      sprintf(msg, "MOUSE UP at (%d, %d)", x, y);
      js_draw_text(20, 770, msg);

      // Draw final marker
      js_set_color("red");
      js_fill_circle(x, y, 6);
    }
    mouse_down = 0;
  }
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

    // Prevent context menu
    canvas.addEventListener('contextmenu', function(e) { e.preventDefault(); });

    console.log('Mouse event listeners set up successfully');
  });
}

// Main demonstration
int main() {
  printf("Advanced Canvas Drawing Demo\n");

  // Clear canvas
  js_clear_canvas();

  // Setup mouse event handling
  setup_mouse_events();
  printf("Mouse events enabled - try drawing on the canvas!\n");

  // Draw title and instructions
  js_set_font("bold 24px Arial");
  js_set_color("darkblue");
  js_draw_text(20, 30, "Advanced Canvas Demo with Mouse Interaction");

  js_set_font("16px Arial");
  js_set_color("black");
  js_draw_text(20, 60, "Click and drag anywhere to draw with cyan lines!");
  js_draw_text(20, 85, "Watch the event log at the bottom of the canvas");

  // Draw grid
  draw_grid(50, "#e0e0e0");

  // Draw coordinate axes
  draw_coordinate_axes(400, 300);

  // Draw function graph
  draw_function_graph(400, 300, 50);

  // Draw bar chart
  int bar_values[] = {120, 80, 150, 90, 110, 130};
  draw_bar_chart(bar_values, 6, 50, 550, 40);

  // Draw pie chart
  int pie_values[] = {30, 20, 25, 15, 10};
  draw_pie_chart(pie_values, 5, 650, 150, 80);

  printf("Drawing complete!\n");

  return 0;
}

// Export function for animation
EMSCRIPTEN_KEEPALIVE
void update_animation() { animate_bouncing_ball(); }
