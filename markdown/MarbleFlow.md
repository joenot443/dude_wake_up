# MarbleFlow

Liquid marble / flowing ink — a soft, chromatically rich field that drifts and folds
like paper marbling. Built by domain-warping a cheap aperiodic "dot noise" (XorDev)
through itself twice, then coloring with a cosine palette. Generated from scratch via
the new-shader-from-scratch skill.

**Parameters**
- **Speed** — flow rate. Integrated as accumulated phase, so changing it eases the motion instead of jumping.
- **Scale** — zoom of the marble (smaller = broader veins).
- **Warp** — domain-warp strength: low = gentle clouds, high = tight turbulent folds.
- **Hue Shift** — rotates the palette.
- **Brightness** — exposure into the soft tone-mapping.

All parameters are audio-reactive via their oscillators.
