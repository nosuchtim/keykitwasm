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
    },

    // ========== Web MIDI API Functions ==========

    // Global MIDI access object
    js_request_midi_access__deps: ['$stringToUTF8'],
    js_request_midi_access: function () {
        if (!window.midiAccess) {
            window.midiInputs = [];
            window.midiOutputs = [];

            if (navigator.requestMIDIAccess) {
                console.log('Requesting MIDI access...');
                navigator.requestMIDIAccess({ sysex: false })
                    .then(function(access) {
                        console.log('MIDI access granted!');
                        window.midiAccess = access;

                        // Store inputs and outputs as arrays
                        window.midiInputs = Array.from(access.inputs.values());
                        window.midiOutputs = Array.from(access.outputs.values());

                        console.log('Found ' + window.midiInputs.length + ' MIDI inputs');
                        console.log('Found ' + window.midiOutputs.length + ' MIDI outputs');

                        // Log device names
                        window.midiInputs.forEach(function(input, index) {
                            console.log('  Input ' + index + ': ' + input.name);
                        });
                        window.midiOutputs.forEach(function(output, index) {
                            console.log('  Output ' + index + ': ' + output.name);
                        });

                        // Call back into C code to redraw MIDI devices
                        if (typeof Module !== 'undefined' && Module.ccall) {
                            Module.ccall('on_midi_ready', null, [], []);
                        }
                    })
                    .catch(function(err) {
                        console.error('MIDI access denied or error:', err);
                        window.midiInputs = [];
                        window.midiOutputs = [];
                    });
            } else {
                console.warn('Web MIDI API not supported in this browser');
                window.midiInputs = [];
                window.midiOutputs = [];
            }
        }
    },

    // Get number of MIDI input devices
    js_get_midi_input_count: function () {
        if (!window.midiInputs) {
            return 0;
        }
        return window.midiInputs.length;
    },

    // Get number of MIDI output devices
    js_get_midi_output_count: function () {
        if (!window.midiOutputs) {
            return 0;
        }
        return window.midiOutputs.length;
    },

    // Get MIDI input device name
    js_get_midi_input_name__deps: ['$stringToUTF8'],
    js_get_midi_input_name: function (index, buffer, buffer_size) {
        if (!window.midiInputs || index < 0 || index >= window.midiInputs.length) {
            stringToUTF8('Unknown', buffer, buffer_size);
            return;
        }
        var device = window.midiInputs[index];
        var name = device.name || 'MIDI Input ' + index;
        stringToUTF8(name, buffer, buffer_size);
    },

    // Get MIDI output device name
    js_get_midi_output_name__deps: ['$stringToUTF8'],
    js_get_midi_output_name: function (index, buffer, buffer_size) {
        if (!window.midiOutputs || index < 0 || index >= window.midiOutputs.length) {
            stringToUTF8('Unknown', buffer, buffer_size);
            return;
        }
        var device = window.midiOutputs[index];
        var name = device.name || 'MIDI Output ' + index;
        stringToUTF8(name, buffer, buffer_size);
    },

    // Open all MIDI input devices and set up message listeners
    js_open_midi_inputs: function () {
        if (!window.midiInputs || window.midiInputs.length === 0) {
            console.log('No MIDI input devices to open');
            return;
        }

        console.log('Opening ' + window.midiInputs.length + ' MIDI input devices...');

        window.midiInputs.forEach(function(input, index) {
            // Set up message handler
            input.onmidimessage = function(event) {
                var data = event.data;
                var status = data[0];
                var data1 = data.length > 1 ? data[1] : 0;
                var data2 = data.length > 2 ? data[2] : 0;

                // Call back into C code with device index and MIDI data
                if (typeof Module !== 'undefined' && Module.ccall) {
                    Module.ccall('on_midi_message', null,
                                 ['number', 'number', 'number', 'number'],
                                 [index, status, data1, data2]);
                }
            };

            console.log('  Opened MIDI input: ' + input.name);
        });

        console.log('All MIDI inputs opened and listening for messages');
    }
});
