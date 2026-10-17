# Silk

An iridescent, draped-satin flow field — soft folds of oil-slick color that drift
and re-form like fabric in a breeze. Generated from scratch (flow-field technique)
via the new-shader-from-scratch skill.

**Parameters**
- **Speed** — animation rate. Integrated as accumulated phase, so changing it eases the motion instead of warping the scene.
- **Scale** — zoom of the flow field (smaller = bigger, broader folds).
- **Flow** — advection step size; how far the silk streaks travel (more = stretchier, wispier).
- **Hue Shift** — rotates the iridescent palette.
- **Sheen** — strength of the satin highlight on the ridges.
- **Brightness** — exposure into the soft tone-mapping.

All parameters are audio-reactive via their oscillators.
