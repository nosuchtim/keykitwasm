// JavaScript Library for Canvas Drawing
// This file defines the JavaScript functions that C code can call
// Use with: emcc --js-library canvas_library.js

mergeInto(LibraryManager.library, {
    // Clear canvas
    js_clear_canvas: function () {
        var canvas = document.getElementById('canvas');
        if (!canvas) {
            console.error('Canvas element not found!');
            return;
        }
        var ctx = canvas.getContext('2d');
        ctx.clearRect(0, 0, canvas.width, canvas.height);
    },

    // Draw line
    js_draw_line: function (x0, y0, x1, y1) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.beginPath();
        ctx.moveTo(x0, y0);
        ctx.lineTo(x1, y1);
        ctx.stroke();
    },

    // Draw rectangle outline
    js_draw_rect: function (x, y, w, h) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.strokeRect(x, y, w, h);
    },

    // Draw filled rectangle
    js_fill_rect: function (x, y, w, h) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.fillRect(x, y, w, h);
    },

    // Draw circle outline
    js_draw_circle: function (x, y, radius) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.beginPath();
        ctx.arc(x, y, radius, 0, 2 * Math.PI);
        ctx.stroke();
    },

    // Draw filled circle
    js_fill_circle: function (x, y, radius) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.beginPath();
        ctx.arc(x, y, radius, 0, 2 * Math.PI);
        ctx.fill();
    },

    // Draw text
    js_draw_text: function (x, y, text) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.fillText(UTF8ToString(text), x, y);
    },

    // Set drawing color
    js_set_color: function (color) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        var colorStr = UTF8ToString(color);
        ctx.strokeStyle = colorStr;
        ctx.fillStyle = colorStr;
    },

    // Set line width
    js_set_line_width: function (width) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.lineWidth = width;
    },

    // Set font
    js_set_font: function (font) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.font = UTF8ToString(font);
    },

    // Get canvas width
    js_get_canvas_width: function () {
        var canvas = document.getElementById('canvas');
        return canvas ? canvas.width : 0;
    },

    // Get canvas height
    js_get_canvas_height: function () {
        var canvas = document.getElementById('canvas');
        return canvas ? canvas.height : 0;
    },

    // Set global alpha (transparency)
    js_set_alpha: function (alpha) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.globalAlpha = alpha;
    },

    // Draw ellipse
    js_draw_ellipse: function (x, y, radiusX, radiusY) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.beginPath();
        ctx.ellipse(x, y, radiusX, radiusY, 0, 0, 2 * Math.PI);
        ctx.stroke();
    },

    // Fill ellipse
    js_fill_ellipse: function (x, y, radiusX, radiusY) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.beginPath();
        ctx.ellipse(x, y, radiusX, radiusY, 0, 0, 2 * Math.PI);
        ctx.fill();
    },

    // Save context state
    js_save_context: function () {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.save();
    },

    // Restore context state
    js_restore_context: function () {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.restore();
    },

    // Set composite operation
    js_set_composite_operation: function (operation) {
        var canvas = document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d');
        ctx.globalCompositeOperation = UTF8ToString(operation);
    }
});
