# MarbleCaves

A single-loop gyroid raymarcher: you fly forward through a translucent marble cave
tunnel with a mirrored "water" surface and a pulsing glow trailing through it. Ported
from "Trailing the Twinkling Tunnelwisp" (CC0).

**Motion**
- **Speed** — how fast you travel through the cave (also drives the glow pulse).
- **Twist** — how tightly the cave walls corkscrew with depth.
- **Steps** — raymarch iterations: more = deeper, more detailed walls, but heavier.

**Cave structure**
- **Detail A / Detail B** — the two gyroid scales combined into the walls (coarse vs fine structure). Pull them apart for more intricate marbling.
- **Density** — wall thickness divisor; lower = denser, brighter, more solid caverns.
- **Translucency** — march epsilon; higher makes the walls hazier / more see-through.

**Look**
- **Reflection** — brightness of the mirrored "water" half below the axis.
- **Color Speed** — how fast the hue cycles as you move through depth.
- **Hue Shift** — overall hue phase offset.
- **Wisp** — brightness of the pulsing tunnelwisp glow.
- **Brightness** — overall exposure (feeds the tanh tone-mapping).

**Audio**
- **Audio Amount** — scales a built-in spectrum effect that maps the live frequency
  spectrum *around the tunnel*: the angle of each wall segment picks a frequency bin, so
  bass blooms one arc of the cave while treble lights another, spiralling as you fly. Set
  to **0** for the pure, non-reactive look; raise for a stronger response. Uses the
  selected audio source's 256-bin spectrum; with no audio it falls back to the plain cave.

All knobs except Steps are also audio-reactive via their oscillators — try driving **Wisp**,
**Density**, or **Brightness** from the beat on top of the spectrum walls.
