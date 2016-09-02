#!/bin/python2
# -*- coding: utf-8 -*-
import cairo
import pango
import pangocairo
import sys

surf = cairo.ImageSurface(cairo.FORMAT_ARGB32, 128, 32)
context = cairo.Context(surf)

#draw a background rectangle:
#context.rectangle(0,0,320,120)
#context.set_source_rgb(1, 1, 1)
#context.fill()

#get font families:

font_map = pangocairo.cairo_font_map_get_default()
families = font_map.list_families()

# to see family names:
print [f.get_name() for f in   font_map.list_families()]

#context.set_antialias(cairo.ANTIALIAS_SUBPIXEL)

pangocairo_context = pangocairo.CairoContext(context)
pangocairo_context.set_antialias(cairo.ANTIALIAS_SUBPIXEL)

layout = pangocairo_context.create_layout()
font = pango.FontDescription("Sans 20")
layout.set_font_description(font)

for i in range(5):
  context.identity_matrix()
  context.translate(i * 128/5,0)
  context.set_source_rgb(255, 255, 255)
  layout.set_text(unichr(32+i))
  print(layout.get_pixel_size())
#  layout.set_text(unichr(0x554c+i))
  pangocairo_context.update_layout(layout)
  pangocairo_context.show_layout(layout)

with open("texture.png", "wb") as image_file:
    surf.write_to_png(image_file)
