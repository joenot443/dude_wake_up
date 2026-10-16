# Audio Suminagashi

Japanese ink marbling on dark water, built on frame feedback. Ink is dropped on the
beat and clear water between beats at the same site, so each new drop pushes the
earlier ink outward into concentric rings — the rhythm is literally recorded in the
rings, the way real suminagashi is made. Three drop sites follow the bass, mids and
highs (fine light-ink accents). Slow currents, a vortex and a breathing shear skew
the rings into eggs and spirals; on a slow clock the surface is combed into feathered
chevrons, swirled with a stylus, or struck by a "stone" of new colour. Every ~36 s a
new palette (indigo/rust, deep jade, ember) re-dyes the sheet along a travelling front.
Inks are immiscible, held crisp by a surface-tension term, with pigment grain and a
reversible washi-grain/vignette display layer. It starts fully marbled on frame one.

**Audio mapping**
- **Beat** — ink commits once per beat at the bass site (beat-tracked), off-beats feed the mid site, 16ths feed the high site; onset strength sizes the drops.
- **Kick** — the whole ring field jumps; louder kicks shove further.
- **Highs** — a shimmering surface ripple.
- Onsets are detected relative to each band's running average, so sparse and heavily compressed tracks both keep their ring/water rhythm.

**Parameters**
- **Drop Force** — radial push of drops (ring spacing).
- **Kick Punch** — strength of the kick impulse.
- **Current** — drift, shear and vortex strength.
- **Gestures** — comb / wave comb / swirl / stone strength (above ~1.5 can smear the whole frame).
- **Shimmer** — hi-hat surface ripple.
- **Separation** — how strongly mixed inks resolve back to pure colours.
- **Tension** — interface sharpening (above ~1.5 risks flooding).
- **Vignette** — vignette, washi grain and caustic sheen.
- **Audio Sensitivity** — onset gain.
