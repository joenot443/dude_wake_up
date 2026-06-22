# Gemfield

A field of glowing, inflated stained-glass gems — an animated Voronoi mosaic that
slowly morphs and breathes. Each cell is shaded brighter toward its center for a
beveled, 3D glass-pebble look, with tinted glowing seams between them. Generated
from scratch (Voronoi cellular technique) via the new-shader-from-scratch skill.

**Parameters**
- **Speed** — rate the cells drift/morph. Integrated as accumulated phase, so changing it eases the motion instead of jumping.
- **Scale** — cell density (higher = smaller, more numerous gems).
- **Seam Glow** — brightness of the glowing borders between cells.
- **Ripple** — depth of the radial breathing pulse across the field.
- **Hue Shift** — rotates the palette.
- **Brightness** — exposure into the soft tone-mapping.

All parameters are audio-reactive via their oscillators.
