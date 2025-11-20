/*
 * EM_ASM QUICK REFERENCE CHEAT SHEET
 *
 * Copy-paste examples for common use cases
 */

#include <emscripten.h>
#include <stdio.h>

// ============================================================================
// BASIC USAGE
// ============================================================================

// 1. Simple JavaScript execution (no parameters, no return)
void example_simple() {
  EM_ASM(console.log('Hello from JavaScript!');
         alert('This is JavaScript code'););
}

// 2. JavaScript with C parameters (no return)
void example_with_params(int x, int y, char *message) {
  EM_ASM(
      {
        console.log('x =', $0);
        console.log('y =', $1);
        console.log('message =', UTF8ToString($2));
      },
      x, y, message);
}

// 3. Get integer from JavaScript
int example_return_int() {
  return EM_ASM_INT({ return 42; });
}

// 4. Get float/double from JavaScript
double example_return_double() {
  return EM_ASM_DOUBLE({ return Math.PI; });
}

// ============================================================================
// CANVAS DRAWING EXAMPLES
// ============================================================================

void canvas_draw_line(int x0, int y0, int x1, int y1) {
  EM_ASM(
      {
        var ctx = document.getElementById('canvas').getContext('2d');
        ctx.beginPath();
        ctx.moveTo($0, $1);
        ctx.lineTo($2, $3);
        ctx.stroke();
      },
      x0, y0, x1, y1);
}

void canvas_draw_rect(int x, int y, int w, int h, char *color) {
  EM_ASM(
      {
        var ctx = document.getElementById('canvas').getContext('2d');
        ctx.fillStyle = UTF8ToString($4);
        ctx.fillRect($0, $1, $2, $3);
      },
      x, y, w, h, color);
}

void canvas_draw_circle(int x, int y, int radius, char *color) {
  EM_ASM(
      {
        var ctx = document.getElementById('canvas').getContext('2d');
        ctx.fillStyle = UTF8ToString($3);
        ctx.beginPath();
        ctx.arc($0, $1, $2, 0, 2 * Math.PI);
        ctx.fill();
      },
      x, y, radius, color);
}

void canvas_draw_text(int x, int y, char *text, char *font, char *color) {
  EM_ASM(
      {
        var ctx = document.getElementById('canvas').getContext('2d');
        ctx.font = UTF8ToString($3);
        ctx.fillStyle = UTF8ToString($4);
        ctx.fillText(UTF8ToString($2), $0, $1);
      },
      x, y, text, font, color);
}

void canvas_clear() {
  EM_ASM({
    var canvas = document.getElementById('canvas');
    var ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);
  });
}

// ============================================================================
// DOM MANIPULATION EXAMPLES
// ============================================================================

void dom_create_element(char *tag, char *id, char *text) {
  EM_ASM(
      {
        var elem = document.createElement(UTF8ToString($0));
        elem.id = UTF8ToString($1);
        elem.textContent = UTF8ToString($2);
        document.body.appendChild(elem);
      },
      tag, id, text);
}

void dom_set_text(char *id, char *text) {
  EM_ASM(
      {
        var elem = document.getElementById(UTF8ToString($0));
        if (elem)
          elem.textContent = UTF8ToString($1);
      },
      id, text);
}

void dom_set_style(char *id, char *property, char *value) {
  EM_ASM(
      {
        var elem = document.getElementById(UTF8ToString($0));
        if (elem)
          elem.style[UTF8ToString($1)] = UTF8ToString($2);
      },
      id, property, value);
}

// ============================================================================
// BROWSER INFO EXAMPLES
// ============================================================================

int get_window_width() {
  return EM_ASM_INT({ return window.innerWidth; });
}

int get_window_height() {
  return EM_ASM_INT({ return window.innerHeight; });
}

int get_canvas_width(char *canvas_id) {
  return EM_ASM_INT(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        return canvas ? canvas.width : 0;
      },
      canvas_id);
}

// ============================================================================
// ARRAY HANDLING EXAMPLES
// ============================================================================

void print_array(int *arr, int length) {
  EM_ASM(
      {
        console.log('Array contents:');
        for (var i = 0; i < $1; i++) {
          var value = getValue($0 + i * 4, 'i32');
          console.log('  [' + i + ']:', value);
        }
      },
      arr, length);
}

void double_array_values(int *arr, int length) {
  EM_ASM(
      {
        for (var i = 0; i < $1; i++) {
          var value = getValue($0 + i * 4, 'i32');
          setValue($0 + i * 4, value * 2, 'i32');
        }
      },
      arr, length);
}

// ============================================================================
// TIMING EXAMPLES
// ============================================================================

void delayed_message(char *message, int delay_ms) {
  EM_ASM(
      { setTimeout(function() { alert(UTF8ToString($0)); }, $1); }, message,
      delay_ms);
}

void repeated_action(int interval_ms) {
  EM_ASM(
      {
        var count = 0;
        setInterval(function() { console.log('Tick', ++count); }, $0);
      },
      interval_ms);
}

// ============================================================================
// ERROR HANDLING EXAMPLES
// ============================================================================

void safe_canvas_operation() {
    EM_ASM({
        try {
            var canvas = document.getElementById('canvas');
            if (!canvas) {
    console.error('Canvas not found!');
    return;
            }
            var ctx = canvas.getContext('2d');
    // ... do something with ctx
}
catch(e) { console.error('Error in canvas operation:', e); }
});
}

// ============================================================================
// COMPLEX EXAMPLES
// ============================================================================

// Draw a gradient rectangle
void draw_gradient_rect(int x, int y, int w, int h, char *color1,
                        char *color2) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');

        var gradient = ctx.createLinearGradient($0, $1, $0 + $2, $1 + $3);
        gradient.addColorStop(0, UTF8ToString($4));
        gradient.addColorStop(1, UTF8ToString($5));

        ctx.fillStyle = gradient;
        ctx.fillRect($0, $1, $2, $3);
      },
      x, y, w, h, color1, color2);
}

// Draw an image
void draw_image(char *image_url, int x, int y, int width, int height) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');

        var img = new Image();
        img.onload = function() { ctx.drawImage(img, $1, $2, $3, $4); };
        img.src = UTF8ToString($0);
      },
      image_url, x, y, width, height);
}

// Get mouse position
typedef struct {
  int x;
  int y;
} Point;

void get_mouse_position(Point *out) {
  EM_ASM(
      {
        var canvas = document.getElementById('canvas');
        var rect = canvas.getBoundingClientRect();

        canvas.addEventListener(
            'mousemove', function(e) {
              var x = e.clientX - rect.left;
              var y = e.clientY - rect.top;
              setValue($0, x, 'i32');
              setValue($0 + 4, y, 'i32');
            });
      },
      out);
}

// ============================================================================
// PARAMETER TYPE REFERENCE
// ============================================================================

void parameter_types_demo() {
  // Integer
  int i = 42;
  EM_ASM({ console.log('int:', $0); }, i);

  // Float/Double
  double d = 3.14159;
  EM_ASM({ console.log('double:', $0); }, d);

  // String
  char *str = "Hello";
  EM_ASM({ console.log('string:', UTF8ToString($0)); }, str);

  // Multiple parameters
  EM_ASM({ console.log('Multiple:', $0, $1, UTF8ToString($2)); }, i, d, str);
}

// ============================================================================
// MEMORY ACCESS REFERENCE
// ============================================================================

void memory_access_demo() {
  int value = 100;
  int *ptr = &value;

  // Read from memory
  int read_value = EM_ASM_INT({ return getValue($0, 'i32'); }, ptr);

  // Write to memory
  EM_ASM({ setValue($0, 200, 'i32'); }, ptr);

  printf("Original: 100, Read: %d, After write: %d\n", read_value, value);
}

// Memory types:
// 'i8'    - 8-bit integer (char)
// 'i16'   - 16-bit integer (short)
// 'i32'   - 32-bit integer (int)
// 'i64'   - 64-bit integer (long long)
// 'float' - 32-bit float
// 'double'- 64-bit double

// ============================================================================
// QUICK REFERENCE TABLE
// ============================================================================

/*
┌─────────────────┬──────────────────────┬─────────────────────────────────┐
│ Macro           │ Use Case             │ Example                         │
├─────────────────┼──────────────────────┼─────────────────────────────────┤
│ EM_ASM          │ No params, no return │ EM_ASM(alert('Hi'));            │
│ EM_ASM_         │ With params          │ EM_ASM_({log($0);}, x);         │
│ EM_ASM_INT      │ Return integer       │ int n = EM_ASM_INT({return 5;}); │
│ EM_ASM_DOUBLE   │ Return float         │ double d = EM_ASM_DOUBLE({...}); │
└─────────────────┴──────────────────────┴─────────────────────────────────┘

Parameter Access:
  $0, $1, $2, ... = First, second, third parameter

String Conversion:
  UTF8ToString($0) = Convert C string pointer to JS string

Memory Access:
  getValue(ptr, 'i32')      = Read from memory
  setValue(ptr, value, 'i32') = Write to memory
*/

// ============================================================================
// MAIN DEMO
// ============================================================================

int main() {
  printf("EM_ASM Examples\n");
  printf("===============\n\n");

  // Simple example
  printf("1. Simple JavaScript:\n");
  EM_ASM(console.log('Hello from EM_ASM!'));

  // With parameters
  printf("2. With parameters:\n");
  example_with_params(10, 20, "Test message");

  // Return value
  printf("3. Return values:\n");
  int width = get_window_width();
  printf("   Window width: %d\n", width);

  // Canvas drawing
  printf("4. Canvas drawing:\n");
  canvas_clear();
  canvas_draw_rect(50, 50, 100, 80, "blue");
  canvas_draw_circle(200, 90, 40, "red");
  canvas_draw_text(50, 150, "Hello Canvas!", "20px Arial", "black");

  printf("\nCheck the browser console and canvas!\n");

  return 0;
}
