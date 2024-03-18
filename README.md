# gst-plugin-tux-audiodelay

## Note

the plugin tested properly on macos with osxaudiosrc but not on linux with pipewiresrc, the code is currently broken while trying to resolve this issue.


## Description

gst-plugin-tux-audiodelay is a simple GStreamer plugin designed for delaying audio streams. 

the device name is called `tux_audio_delay` with a property of `delay_ms` for delay in milliseconds that supports up to 30 seconds.

this is my first plugin i ever wrote, it supposed to support changing delay_ms in real time, didn't fully test it yet, i'm sure that it still have lots of improvements to be done, but for now the basic functionality does work.


## Note

As a beginner in plugin development, I appreciate your patience and understanding as I work through the learning process. Contributions, feedback, and suggestions are always welcome as I strive to improve and expand the capabilities of this plugin.

Thank you for your interest in gst-plugin-tux-audiodelay!

