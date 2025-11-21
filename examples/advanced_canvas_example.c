// Method 2: Using JavaScript Library Functions (More Efficient)
// This approach pre-defines JavaScript functions and calls them from C

#include <emscripten.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

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

// ========== MIDI DEVICE LISTING (Web MIDI API) ==========

// External JavaScript functions for Web MIDI API
extern int js_get_midi_input_count();
extern int js_get_midi_output_count();
extern void js_get_midi_input_name(int index, char *buffer, int buffer_size);
extern void js_get_midi_output_name(int index, char *buffer, int buffer_size);
extern void js_request_midi_access();
extern void js_open_midi_inputs();

// Get number of MIDI input devices (calls Web MIDI API)
int get_midi_input_count() {
    return js_get_midi_input_count();
}

// Get number of MIDI output devices (calls Web MIDI API)
int get_midi_output_count() {
    return js_get_midi_output_count();
}

// Get MIDI input device name by index
void get_midi_input_name(int index, char *buffer, int buffer_size) {
    js_get_midi_input_name(index, buffer, buffer_size);
}

// Get MIDI output device name by index
void get_midi_output_name(int index, char *buffer, int buffer_size) {
    js_get_midi_output_name(index, buffer, buffer_size);
}

// Request MIDI access from browser
void request_midi_access() {
    js_request_midi_access();
}

// Draw MIDI device list on canvas in two columns
void draw_midi_devices(int x, int y) {
    js_set_font("bold 16px Arial");
    js_set_color("darkgreen");
    js_draw_text(x, y, "MIDI Devices:");

    int column_width = 380;
    int input_x = x + 10;
    int output_x = x + column_width + 20;
    int current_y = y + 25;

    // Draw MIDI Inputs (Left Column)
    js_set_font("bold 14px Arial");
    js_set_color("blue");
    js_draw_text(input_x, current_y, "Inputs:");

    js_set_font("12px monospace");
    js_set_color("black");

    int input_y = current_y + 20;
    int input_count = get_midi_input_count();
    if (input_count == 0) {
        js_set_color("gray");
        js_draw_text(input_x, input_y, "  No MIDI inputs found");
        input_y += 18;
    } else {
        for (int i = 0; i < input_count; i++) {
            char device_name[256];
            char label[300];
            get_midi_input_name(i, device_name, sizeof(device_name));
            sprintf(label, "  [%d] %s", i, device_name);
            js_draw_text(input_x, input_y, label);
            input_y += 18;
        }
    }

    // Draw MIDI Outputs (Right Column)
    js_set_font("bold 14px Arial");
    js_set_color("blue");
    js_draw_text(output_x, current_y, "Outputs:");

    js_set_font("12px monospace");
    js_set_color("black");

    int output_y = current_y + 20;
    int output_count = get_midi_output_count();
    if (output_count == 0) {
        js_set_color("gray");
        js_draw_text(output_x, output_y, "  No MIDI outputs found");
        output_y += 18;
    } else {
        for (int i = 0; i < output_count; i++) {
            char device_name[256];
            char label[300];
            get_midi_output_name(i, device_name, sizeof(device_name));
            sprintf(label, "  [%d] %s", i, device_name);
            js_draw_text(output_x, output_y, label);
            output_y += 18;
        }
    }

    // Draw borders around each column
    js_set_color("darkgreen");
    js_set_line_width(2);

    // Calculate box heights for each column
    int input_height = input_y - current_y + 10;
    int output_height = output_y - current_y + 10;
    int max_height = input_height > output_height ? input_height : output_height;

    // Draw overall border
    js_draw_rect(x - 5, y - 20, column_width * 2 + 35, max_height);

    // Draw vertical divider between columns
    js_set_line_width(1);
    js_draw_line(x + column_width + 10, current_y - 5, x + column_width + 10, y + max_height - 20);
}

// ========== MIDI MESSAGE DISPLAY ==========

#define MAX_MIDI_MESSAGES 10
static char midi_messages[MAX_MIDI_MESSAGES][256];
static int midi_message_count = 0;

// Forward declaration
void draw_midi_message_area();

// Add a MIDI message to the display buffer
void add_midi_message(const char *msg) {
  // Shift messages up
  for (int i = MAX_MIDI_MESSAGES - 1; i > 0; i--) {
    strcpy(midi_messages[i], midi_messages[i-1]);
  }

  // Add new message at top
  strncpy(midi_messages[0], msg, 255);
  midi_messages[0][255] = '\0';

  if (midi_message_count < MAX_MIDI_MESSAGES) {
    midi_message_count++;
  }

  // Redraw the MIDI message area
  draw_midi_message_area();
}

// Draw the MIDI message display area
void draw_midi_message_area() {
  int x = 20;
  int y = 340;
  int width = 760;
  int height = 220;

  // Clear the area with a dark background
  js_set_color("rgba(0, 0, 0, 0.85)");
  js_fill_rect(x, y, width, height);

  // Draw border
  js_set_color("lime");
  js_set_line_width(2);
  js_draw_rect(x, y, width, height);

  // Draw title
  js_set_font("bold 14px monospace");
  js_set_color("lime");
  js_draw_text(x + 10, y + 20, "MIDI MESSAGES (Last 10):");

  // Draw messages
  js_set_font("12px monospace");
  for (int i = 0; i < midi_message_count; i++) {
    int msg_y = y + 45 + i * 18;
    js_draw_text(x + 15, msg_y, midi_messages[i]);
  }

  // Draw "waiting" message if no messages yet
  if (midi_message_count == 0) {
    js_set_color("gray");
    js_draw_text(x + 15, y + 45, "Waiting for MIDI input...");
  }
}

// Called from JavaScript when a MIDI message is received
EMSCRIPTEN_KEEPALIVE
void on_midi_message(int device_index, int status, int data1, int data2) {
  char msg[256];
  char device_name[128];

  get_midi_input_name(device_index, device_name, sizeof(device_name));

  // Decode MIDI message type
  int msg_type = (status >> 4) & 0x0F;
  int channel = status & 0x0F;

  const char *type_str = "Unknown";

  if (msg_type == 0x08) {
    type_str = "Note Off";
    sprintf(msg, "[%s] Ch%d: %s Note=%d Vel=%d",
            device_name, channel + 1, type_str, data1, data2);
  } else if (msg_type == 0x09) {
    if (data2 == 0) {
      type_str = "Note Off";
    } else {
      type_str = "Note On";
    }
    sprintf(msg, "[%s] Ch%d: %s Note=%d Vel=%d",
            device_name, channel + 1, type_str, data1, data2);
  } else if (msg_type == 0x0A) {
    type_str = "Aftertouch";
    sprintf(msg, "[%s] Ch%d: %s Note=%d Pressure=%d",
            device_name, channel + 1, type_str, data1, data2);
  } else if (msg_type == 0x0B) {
    type_str = "CC";
    sprintf(msg, "[%s] Ch%d: %s Controller=%d Value=%d",
            device_name, channel + 1, type_str, data1, data2);
  } else if (msg_type == 0x0C) {
    type_str = "Program";
    sprintf(msg, "[%s] Ch%d: %s Program=%d",
            device_name, channel + 1, type_str, data1);
  } else if (msg_type == 0x0D) {
    type_str = "Chan Pressure";
    sprintf(msg, "[%s] Ch%d: %s Pressure=%d",
            device_name, channel + 1, type_str, data1);
  } else if (msg_type == 0x0E) {
    type_str = "Pitch Bend";
    int bend_value = (data2 << 7) | data1;
    sprintf(msg, "[%s] Ch%d: %s Value=%d",
            device_name, channel + 1, type_str, bend_value);
  } else {
    sprintf(msg, "[%s] Status=0x%02X Data1=%d Data2=%d",
            device_name, status, data1, data2);
  }

  add_midi_message(msg);
  printf("MIDI: %s\n", msg);
}

// ========== CALLBACK FOR MIDI READY ==========

// Called from JavaScript when MIDI devices are ready
EMSCRIPTEN_KEEPALIVE
void on_midi_ready() {
  printf("MIDI devices ready! Redrawing MIDI device list...\n");
  printf("Found %d MIDI inputs and %d MIDI outputs\n",
         get_midi_input_count(), get_midi_output_count());

  // Redraw the MIDI device section at top
  draw_midi_devices(20, 140);

  // Open all MIDI inputs
  js_open_midi_inputs();
  printf("Opening all MIDI input devices for monitoring...\n");

  // Draw initial MIDI message area
  draw_midi_message_area();
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

  // Request MIDI access from browser
  request_midi_access();
  printf("Requesting MIDI access...\n");

  // Clear canvas
  js_clear_canvas();

  // Setup mouse event handling
  setup_mouse_events();
  printf("Mouse events enabled - try drawing on the canvas!\n");

  // Draw title and instructions
  js_set_font("bold 24px Arial");
  js_set_color("darkblue");
  js_draw_text(20, 30, "Advanced Canvas Demo with MIDI & Mouse");

  js_set_font("16px Arial");
  js_set_color("black");
  js_draw_text(20, 60, "All MIDI inputs are monitored - play notes/send MIDI to see messages!");
  js_draw_text(20, 85, "Click and drag to draw with cyan lines");
  js_draw_text(20, 110, "Web MIDI devices are listed below");

  // Draw MIDI device list at top (initially will show "No devices found")
  draw_midi_devices(20, 140);

  // Draw grid
  draw_grid(50, "#e0e0e0");

  // Draw coordinate axes (moved down to make room for MIDI devices)
  draw_coordinate_axes(400, 450);

  // Draw function graph
  draw_function_graph(400, 450, 50);

  // Draw bar chart (moved down)
  int bar_values[] = {120, 80, 150, 90, 110, 130};
  draw_bar_chart(bar_values, 6, 50, 700, 40);

  // Draw pie chart
  int pie_values[] = {30, 20, 25, 15, 10};
  draw_pie_chart(pie_values, 5, 650, 200, 80);

  printf("Drawing complete!\n");
  printf("Waiting for MIDI devices to be enumerated...\n");

  return 0;
}

// Export function for animation
EMSCRIPTEN_KEEPALIVE
void update_animation() { animate_bouncing_ball(); }
