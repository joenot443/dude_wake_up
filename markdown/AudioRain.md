# Audio Rain

Rain on the side window of a car driving through a city at night. Outside, the
out-of-focus world slides past in parallax: a far skyline drifting slowly, shopfronts,
signs and big neon tube signs at middle distance, oncoming headlights smearing by on the
wet road, and roadside lamps whipping past close to the glass. Overhead streetlights sweep
a warm wash across the window as each one passes, and the drops sparkle as the light goes
over them. On the glass, beads and drops act as tiny lenses; at speed the airflow pushes
the water back along the window in diagonal streaks, and stopped or parked it slides
straight down.

**The drive** — a ~4 minute loop (Scene = Auto): downtown neon, sodium-lit residential
streets, a highway with lane cat-eyes, trucks and overhead gantry signs, and a tunnel of
rhythmic orange lights, then back downtown. On city streets the car brakes for red lights:
the scenery stops, the rain runs straight down, the signal glows red, turns green, and the
car pulls away. Locking a Scene dissolves to it over ~2 s.

**Audio mapping**
- **Kick** — the lights pulse and the lens "breathes" focus; strong kicks can send an
  overtaking car's headlights sweeping along the glass, and (with Lightning high) a flash
  of distant sheet lightning. Kicks are detected relative to the track's own running bass
  level, so compressed masters still read.
- **Bass level** — gentle speed surges (eases on with the bass, coasts when it drops).
- **Mids** — how hard it rains. **Highs** — fine spatter and glints on the drops.

**Parameters**
- **Drive Speed** — 0 parks the car (still view, route pauses); higher drives faster.
- **Direction** — which side window: mirrors the drive (scenery, streaks, interior).
- **Scene** — 0 Auto route, 1 Downtown, 2 Residential, 3 Highway, 4 Tunnel (crossfades).
- **Route Pace** — how quickly Auto moves through the areas.
- **Red Lights** — how strongly the car stops at lights (0 = never stops).
- **Parallax Depth** — how much faster near things pass than far ones.
- **Traffic** — passing cars and highway trucks.
- **Streetlights** — overhead streetlight sweeps and their wash over the glass.
- **Headlight Sweeps** — how often / how bright the kick-triggered overtaking headlights are.
- **Lightning** — occasional distant sheet-lightning flashes (strong kicks can trigger them when high).
- **Rain Amount**, **Wind Streak** (airflow push), **Drop Size**, **Trail Length** — the water on the glass.
- **Window Fog** — condensation haze that softens the view; drops and trails clear paths through it.
- **Fog Wipe** — a hand-wiped clear patch on the glass.
- **Focus**, **Bokeh Size**, **Aperture Blades** (4 = round, 5–8 = polygonal; shapes morph) — the lens.
- **City Glow**, **Neon Mix** — brightness of the lights; coloured neon vs white/sodium.
- **Interior Glow** — dashboard, instrument rings and a phone screen reflected on the inside of the glass.
- **Color Temperature**, **Saturation**, **Hue Shift**, **Vignette**, **Film Grain** — the grade.
- **Kick Punch** — strength of the kick pulse and headlight sweeps.
- **Audio Sensitivity** — gain on kick detection, speed surges and the mids/highs response.
- **Brightness** — exposure into the soft tone-mapping.
