// The Nottawa promo: real footage from the app (capture.sh renders each shot frame by frame,
// reacting to the track), composed by seek(t) in a headless page, then muxed with the track.
// 120 BPM: every cut and entrance lands on a beat of the track.
//
//   node scripts/video/promo.mjs plan <wide|tall> > plan.json        # what capture.sh renders
//   scripts/video/capture.sh plan.json <footageDir>
//   node scripts/video/promo.mjs render <wide|tall> <footageDir> <out.mp4> [track.mp3]
//   STILLS=4.2,9,17 node scripts/video/promo.mjs render …          # just those moments, as PNGs
//
// Video beat b runs at 2 per second; the track plays from START, so track time = START + b / 2.

import { execFileSync } from 'node:child_process';
import { existsSync, mkdirSync, readdirSync, rmSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { pathToFileURL } from 'node:url';

const HERE = dirname(new URL(import.meta.url).pathname);
const FPS = 30;
const START = 36;              // track seconds at video 0: four bars before the drop at 40
const STOP = 80;               // just before the track's next hit; the logo holds in silence after
const DURATION = 45;
const LATER = 24;              // beats the closing sections (grid onward) come after the effects and Feedback
const FORMATS = { wide: { W: 1920, H: 1080 }, tall: { W: 1080, H: 1920 } };
const tr = (b) => START + b / 2;

// ── The shots ────────────────────────────────────────────────────────────────────────────
// `chain`: a source (name, or "#type"), then effects. `name` is what the video calls it.
const HERO = { chain: ['MarbleCaves'], name: 'MarbleCaves' };
const MONTAGE = [            // one per beat, b16–24
  { chain: ['ElectricEels'], name: 'ElectricEels' },
  { chain: ['Audio Cymatics'], name: 'Audio Cymatics' },
  { chain: ['Gemfield'], name: 'Gemfield' },
  { chain: ['Glowing Rings'], name: 'Glowing Rings' },
  { chain: ['Melter'], name: 'Melter' },
  { chain: ['Accretion'], name: 'Accretion' },
  { chain: ['Bubbles'], name: 'Bubbles' },
  { chain: ['#52'], name: 'Fractal' },
];
const CHAIN = { chain: ['Plasma Two', 'Kaleidoscope', 'Neon'], names: ['Plasma Two', 'Kaleidoscope', 'Neon'] };
const FX = {                 // b36–44: one source, a different effect on each beat
  source: 'MarbleCaves',
  effects: ['Crystal Kaleidoscope', 'RuttEtra', 'Cubify', 'Comicbook', 'Canny', 'Half Tone', 'Solar Wave', 'Web'],
};
// b44–60: Feedback. The first look also plays in the loop diagram (b44–48), then each fills the frame.
// Node 0 is the source, 1 Feedback, 2+ effects inside its loop (its aux output).
const fbBase = [
  { node: 1, name: 'Delay Amount', value: 28 }, { node: 1, name: 'Feedback Alpha', value: 1 },
  { node: 1, name: 'Feedback Mix', value: 0.92 }, { node: 1, name: 'Blend Mode', value: 4 },
];
const FEEDBACK = [
  { chain: ['Core', 'Feedback'], label: 'Tunnel', settings: [...fbBase, { node: 1, name: 'Scale', value: 1.03 }] },
  { chain: ['Iridescent', 'Feedback'], label: 'Spiral', settings: [...fbBase, { node: 1, name: 'Scale', value: 1.025 }, { node: 1, name: 'Rotation', value: 0.035 }] },
  { chain: ['Swirling Soul', 'Feedback'], label: 'Kaleidoscope in the loop', aux: { from: 1, chain: ['Kaleidoscope'] },
    settings: [...fbBase.slice(0, 2), { node: 1, name: 'Feedback Mix', value: 0.85 }, fbBase[3], { node: 1, name: 'Scale', value: 1.02 }] },
  // Scale on the beat: none between beats (the pulse rests near 0), a step out on each one.
  // It takes a few beats to build: it starts 4 s early.
  { chain: ['Reflector', 'Feedback'], label: 'Hue in the loop, on the beat', aux: { from: 1, chain: ['HSB'] }, warmup: 4,
    settings: [...fbBase, { node: 1, name: 'Scale', driver: 'BPM', shift: 0.53, scale: 0.6 }, { node: 2, name: 'Hue', value: 0.03 }] },
];
const GRID = [               // b36–40 (+LATER), one more per beat
  { chain: ['Audio Rain'], name: 'Audio Rain' },
  { chain: ['Synthwave'], name: 'Synthwave' },
  { chain: ['Audio Silk'], name: 'Audio Silk' },
  { chain: ['AudioBall'], name: 'AudioBall' },
];
const QUICK = [              // a cut every half beat, b40–44 (+LATER)
  { chain: ['DiscoSun'] }, { chain: ['Disco'] }, { chain: ['Perplexion'] }, { chain: ['Smoke Ring'] },
  { chain: ['Reflector'] }, { chain: ['Core'] }, { chain: ['Audio Prism'] }, { chain: ['Blocks'] },
];
const HERO_B = { chain: ['Audio Silk'], name: 'Audio Silk' };     // b44–48 (+LATER), under the words
const HERO_C = { chain: ['Gyroids'], name: 'Gyroids' };           // b48–56 (+LATER)

// Each footage clip: its chain, the track time it starts at, its length, its size (1 = full).
function clips() {
  const out = [];
  const add = (key, shot, b0, b1, extra = {}) => out.push({ key, chain: shot.chain, start: tr(b0), seconds: (b1 - b0) / 2, ...extra });
  add('hero', HERO, 0, 16);
  MONTAGE.forEach((s, i) => add(`m${i}`, s, 16 + i, 17 + i, { warmup: 0.5 }));
  add('chain', CHAIN, 24, 36, { keep: [0, 1, 2] });
  FX.effects.forEach((e, i) => add(`fx${i}`, { chain: [FX.source, e] }, 36 + i, 37 + i, { warmup: 0.5 }));
  // Feedback builds up over its first frames: a longer warm-up, at twice the frame rate.
  const fb = (s) => ({ warmup: s.warmup ?? 2, substeps: 2, settings: s.settings, ...(s.aux ? { aux: s.aux } : {}) });
  add('fb0', FEEDBACK[0], 44, 51, { ...fb(FEEDBACK[0]), keep: [0, 1] });
  FEEDBACK.slice(1).forEach((s, i) => add(`fb${i + 1}`, s, 51 + 3*i, 54 + 3*i, fb(s)));
  const L = LATER;
  GRID.forEach((s, i) => add(`g${i}`, s, L + 36, L + 40, { scale: 0.5 }));
  QUICK.forEach((s, i) => add(`q${i}`, s, L + 40 + i / 2, L + 40.5 + i / 2, { warmup: 0.5 }));
  add('heroB', HERO_B, L + 44, L + 48);
  add('heroC', HERO_C, L + 48, L + 56);
  return out;
}

function plan(format) {
  const { W, H } = FORMATS[format];
  const shots = clips().map(({ key, start, seconds, scale = 1, ...rest }) => ({
    name: key, start, seconds, ...rest, width: Math.round(W * scale), height: Math.round(H * scale),
  }));
  return { fps: FPS, width: W, height: H, shots };
}

// ── The composition ──────────────────────────────────────────────────────────────────────
function page(format, footage) {
  const { W, H } = FORMATS[format];
  // A clip's frame dirs: "<key>" or, for a kept chain node, "<key>-n<i>".
  const dirs = {};
  for (const d of readdirSync(footage, { withFileTypes: true })) {
    if (!d.isDirectory()) continue;
    const n = readdirSync(resolve(footage, d.name)).filter((f) => f.endsWith('.jpg')).length;
    dirs[d.name] = { url: pathToFileURL(resolve(footage, d.name)).href, n };
  }
  const starts = Object.fromEntries(clips().map((c) => [c.key, c.start]));
  const CFG = {
    W, H, FPS, START, tall: format === 'tall', dirs, starts,
    hero: HERO.name, montage: MONTAGE.map((s) => s.name), chainNames: CHAIN.names, grid: GRID.map((s) => s.name),
    heroB: HERO_B.name, heroC: HERO_C.name, later: LATER,
    fxSource: FX.source, fx: FX.effects, fbLabels: FEEDBACK.map((s) => s.label), fbSources: FEEDBACK.map((s) => s.chain[0]),
    font: pathToFileURL(resolve(HERE, 'fonts/Geist-Variable.woff2')).href,
    mono: pathToFileURL(resolve(HERE, 'fonts/GeistMono-Variable.woff2')).href,
  };
  return `<!doctype html><html><head><meta charset="utf-8"><style>
@font-face{font-family:G;src:url(${CFG.font}) format('woff2');font-weight:100 900}
@font-face{font-family:GM;src:url(${CFG.mono}) format('woff2');font-weight:100 900}
*{box-sizing:border-box;margin:0}
html,body{width:${W}px;height:${H}px;overflow:hidden;background:#08080c;font-family:G,sans-serif;color:#f5f3f0;-webkit-font-smoothing:antialiased}
.abs{position:absolute}
.fill{position:absolute;inset:0}
.img{position:absolute;inset:0;width:100%;height:100%;object-fit:cover}
.mono{font-family:GM,monospace;text-transform:uppercase;letter-spacing:.14em;font-weight:500}
.chip{position:absolute;display:flex;align-items:baseline;gap:18px;padding:22px 32px;border-radius:24px;background:rgba(14,14,20,.62);backdrop-filter:blur(24px) saturate(1.4);border:1px solid rgba(255,255,255,.12);box-shadow:0 20px 60px rgba(0,0,0,.45);white-space:nowrap}
.num{font-size:64px;font-weight:700;letter-spacing:-.03em;font-variant-numeric:tabular-nums;line-height:1}
.lbl{font-size:28px;font-weight:500;color:rgba(245,243,240,.72)}
.word{position:absolute;font-weight:750;letter-spacing:-.045em;line-height:1;white-space:nowrap}
.card{position:absolute;overflow:hidden;border-radius:18px;background:#000;box-shadow:0 0 0 1.5px rgba(255,255,255,.14),0 30px 90px rgba(0,0,0,.6)}
.card .title{position:absolute;left:0;right:0;bottom:0;padding:40px 22px 16px;background:linear-gradient(transparent,rgba(0,0,0,.78));font-size:26px;font-weight:600;letter-spacing:-.01em}
.grain{position:absolute;inset:-64px;opacity:.06;mix-blend-mode:screen;pointer-events:none}
</style></head><body><div id="root" class="fill"></div><script>
const C = ${JSON.stringify(CFG)};
const root = document.getElementById('root');
const ACCENT = '#d9653a';    // the site's #B55126, lifted for a dark stage
const clamp = (x, a = 0, b = 1) => Math.max(a, Math.min(b, x));
const lerp = (a, b, u) => a + (b - a) * u;
const easeOut = (x) => 1 - Math.pow(1 - clamp(x), 3);
const easeIn = (x) => Math.pow(clamp(x), 3);
const easeInOut = (x) => { x = clamp(x); return x < .5 ? 4*x*x*x : 1 - Math.pow(-2*x + 2, 3) / 2; };
// A spring with a small overshoot (no wobble), 0 → 1.
const spring = (x) => { x = clamp(x); return 1 - Math.exp(-7*x) * Math.cos(6.2*x) * (1 - x*0.15); };
const seg = (b, from, to) => clamp((b - from) / (to - from));
const S = Math.min(C.W, C.H) / 1080;   // type scale: the short side

// Footage: the frame of clip \`key\` at track time \`t\`.
const need = [];
function src(key, t) {
  const d = C.dirs[key]; if (!d) return '';
  const base = C.starts[key.replace(/-n\\d+$/, '')];
  const i = Math.max(0, Math.min(d.n - 1, Math.round((t - base) * C.FPS)));
  const s = d.url + '/' + String(i).padStart(5, '0') + '.jpg';
  need.push(s); return s;
}
const full = (key, t, style = '') => '<img class="img" src="' + src(key, t) + '" style="' + style + '">';
// A node as the editor draws it: the picture fills it, title over a gradient, pins on its edges.
// Pins on its left and right edges, or (vertical, for a chain running down) its top and bottom.
function node(key, t, x, y, w, h, title, show = 1, extra = '', vertical = false) {
  if (show <= 0) return '';
  const k = spring(show);
  const pin = (side) => '<div class="abs" style="'+side+':-9px;'+(vertical ? 'left:calc(50% - 9px)' : 'top:calc(50% - 9px)')+';width:18px;height:18px;border-radius:50%;background:'+ACCENT+';box-shadow:0 0 0 3px #08080c"></div>';
  return '<div class="abs" style="left:'+x+'px;top:'+y+'px;width:'+w+'px;height:'+h+'px;opacity:'+clamp(show*2)+';transform:scale('+(.82+.18*k)+');'+extra+'">'
    + '<div class="card fill"><img class="img" src="'+src(key, t)+'">' + (title ? '<div class="title">'+title+'</div>' : '') + '</div>'
    + (vertical ? pin('top') + pin('bottom') : pin('left') + pin('right')) + '</div>';
}
const chip = (x, y, num, lbl, show, anchor = 'left') => show <= 0 ? '' : '<div class="chip" style="'+anchor+':'+x+'px;top:'+y+'px;opacity:'+clamp(show*1.6)+';transform:translateY('+((1-spring(show))*40)+'px) scale('+(.9+.1*spring(show))+');transform-origin:'+anchor+' center"><span class="num">'+num+'</span><span class="lbl">'+lbl+'</span></div>';
const label = (text, show = 1) => show <= 0 ? '' : '<div class="abs mono" style="left:'+(48*S)+'px;bottom:'+(48*S)+'px;padding:'+(12*S)+'px '+(20*S)+'px;border-radius:'+(14*S)+'px;background:rgba(8,8,12,.66);backdrop-filter:blur(16px);font-size:'+(22*S)+'px;color:rgba(245,243,240,.92);opacity:'+show+'"><span style="color:'+ACCENT+'">●</span>&nbsp; '+text+'</div>';
const flash = (b, at, len = .35) => { const u = seg(b, at, at + len); return u <= 0 || u >= 1 ? '' : '<div class="fill" style="background:#fff;opacity:'+(.55*(1-easeOut(u)))+';mix-blend-mode:screen"></div>'; };
const countUp = (n, u) => Math.round(n * easeOut(u));

// Grain: a few fixed noise tiles, one per frame in turn.
const grain = [...Array(6)].map((_, k) => {
  const c = document.createElement('canvas'); c.width = c.height = 256;
  const g = c.getContext('2d'); const d = g.createImageData(256, 256);
  let s = 1234 + k * 977;
  for (let i = 0; i < d.data.length; i += 4) { s = (s * 16807) % 2147483647; const v = 128 + ((s % 255) - 127) * .9; d.data[i] = d.data[i+1] = d.data[i+2] = v; d.data[i+3] = 255; }
  g.putImageData(d, 0, 0); return c.toDataURL();
});
const cache = new Map();
function img(s){ if (!cache.has(s)) { const i = new Image(); i.src = s; cache.set(s, i.decode().catch(() => {})); } return cache.get(s); }

window.seek = async (t) => {
  const b = t * 2, T = C.START + t, fi = Math.round(t * C.FPS);
  need.length = 0;
  let out = '';
  const W = C.W, H = C.H;

  // ── A. One node (0–8): the hero in a node card; the tagline lands a word per beat ─────
  // ── B. The drop (8): the card opens to the whole frame ────────────────────────────────
  if (b < 16) {
    const cw = C.tall ? W * .62 : W * .38, ch = cw * H / W;
    const push = easeIn(seg(b, 6, 8));                  // the riser: the card leans in
    const open = easeInOut(seg(b, 7.6, 8));             // …and fills the frame on the drop
    const w = lerp(cw * lerp(1, 1.18, push), W, open), h = lerp(ch * lerp(1, 1.18, push), H, open);
    const cy = C.tall ? H * .4 : H * .42;
    const x = lerp(W/2 - w/2, 0, open), y = lerp(cy - h/2, 0, open);
    const r = lerp(18, 0, open);
    if (b < 8) {
      out += node('hero', T, x, y, w, h, open < .2 ? C.hero : '', seg(b, 0, 1));
      // Live / visuals / for / everyone.
      const words = ['Live', 'visuals', 'for', 'everyone.'];
      const size = (C.tall ? 120 : 112) * S;
      let line = '';
      words.forEach((wd, k) => {
        const u = seg(b, 4 + k, 4.6 + k);
        if (u <= 0) return;
        const v = spring(u);
        line += '<span style="display:inline-block;margin:0 '+(size*.12)+'px;opacity:'+clamp(u*2)+';transform:translateY('+((1-v)*size*.5)+'px)'+(k === 3 ? ';color:'+ACCENT : '')+'">'+wd+'</span>';
      });
      const ty = C.tall ? cy + ch/2 + 140*S : cy + ch/2 + 90*S;
      out += '<div class="abs" style="left:0;right:0;top:'+ty+'px;text-align:center;font-size:'+size+'px;font-weight:750;letter-spacing:-.045em;opacity:'+(1-open)+'">'+line+'</div>';
      out += '<div class="abs mono" style="left:0;right:0;top:'+(C.tall ? 220*S : 110*S)+'px;text-align:center;font-size:'+(24*S)+'px;color:rgba(245,243,240,.6);opacity:'+(easeOut(seg(b, .5, 1.5))*(1-open))+'">Nottawa</div>';
    } else {
      out += full('hero', T);
      out += flash(b, 8);
      // The numbers, one per bar-half.
      const cx = C.tall ? 56*S : 72*S;
      const top = C.tall ? H * .16 : H * .14, gap = 128 * S;
      out += chip(cx, top, countUp(90, seg(b, 10, 10.8)), 'generators', seg(b, 10, 10.6));
      out += chip(cx, top + gap, countUp(75, seg(b, 11, 11.8)), 'effects', seg(b, 11, 11.6));
      out += chip(cx, top + 2*gap, '♪', 'react to any audio', seg(b, 12, 12.6));
      out += label(C.hero, easeOut(seg(b, 9, 10)));
    }
  }

  // ── C. Montage (16–24): a new source on every beat ─────────────────────────────────────
  if (b >= 16 && b < 24) {
    const k = Math.floor(b - 16);
    out += full('m' + k, T, 'transform:scale('+(1.06 - .06*easeOut(seg(b, 16+k, 17+k)))+')');
    out += label(C.montage[k] + '<span style="opacity:.5">&nbsp;&nbsp;' + String(k+1).padStart(2,'0') + '/08</span>');
    out += flash(b, 16 + k, .25);
  }

  // ── D. A chain (24–36): source → effect → effect, then its output takes the frame ───────
  if (b >= 24 && b < 36) {
    // The editor's dot grid.
    out += '<div class="fill" style="background-image:radial-gradient(rgba(255,255,255,.09) 1.6px,transparent 1.6px);background-size:'+(40*S)+'px '+(40*S)+'px"></div>';
    const grow = easeInOut(seg(b, 32, 33.2));
    // Wide: left to right. Tall: down the left side, the words in the column beside it.
    const nw = C.tall ? W * .3 : W * .27, nh = nw * H / W, gap = C.tall ? 40*S : 100*S;
    const pos = [0, 1, 2].map((i) => C.tall
      ? { x: W * .07, y: (H - 3*nh - 2*gap) / 2 + i * (nh + gap) }
      : { x: W/2 - nw/2 + (i - 1) * (nw + gap), y: H/2 - nh/2 + 30*S });
    // Cables: from a node's output pin to the next one's input pin.
    let svg = '';
    for (let i = 0; i < 2; i++) {
      const u = easeInOut(seg(b, 25 + 2*i, 26 + 2*i));
      if (u <= 0) continue;
      const a = C.tall ? { x: pos[i].x + nw/2, y: pos[i].y + nh } : { x: pos[i].x + nw, y: pos[i].y + nh/2 };
      const z = C.tall ? { x: pos[i+1].x + nw/2, y: pos[i+1].y } : { x: pos[i+1].x, y: pos[i+1].y + nh/2 };
      const d = C.tall
        ? 'M'+a.x+' '+a.y+' L'+z.x+' '+z.y
        : 'M'+a.x+' '+a.y+' C'+(a.x+80*S)+' '+a.y+' '+(z.x-80*S)+' '+z.y+' '+z.x+' '+z.y;
      svg += '<path d="'+d+'" fill="none" stroke="'+ACCENT+'" stroke-width="'+(5*S)+'" stroke-linecap="round" pathLength="1" stroke-dasharray="1" stroke-dashoffset="'+(1-u)+'"/>';
    }
    out += '<svg class="fill" width="'+W+'" height="'+H+'" style="opacity:'+(1-grow)+'">'+svg+'</svg>';
    for (let i = 0; i < 3; i++) {
      const show = seg(b, 24 + 2*i, 24.6 + 2*i);
      const p = pos[i];
      if (i < 2) { out += '<div style="opacity:'+(1-grow)+'">' + node('chain-n'+i, T, p.x, p.y, nw, nh, C.chainNames[i], show, '', C.tall) + '</div>'; continue; }
      // The last one grows into the frame.
      const w = lerp(nw, W, grow), h = lerp(nh, H, grow);
      out += node('chain-n2', T, lerp(p.x, 0, grow), lerp(p.y, 0, grow), w, h, grow < .2 ? C.chainNames[2] : '', show, 'transform:none', C.tall);
    }
    // Kinds: under the nodes, or beside them.
    const kinds = ['Source', 'Effect', 'Effect'];
    for (let i = 0; i < 3; i++) {
      const u = easeOut(seg(b, 24.4 + 2*i, 25 + 2*i));
      const p = pos[i];
      const at = C.tall ? 'left:'+(p.x + nw + 40*S)+'px;top:'+(p.y + 24*S)+'px' : 'left:'+p.x+'px;width:'+nw+'px;top:'+(p.y + nh + 22*S)+'px;text-align:center';
      out += '<div class="abs mono" style="'+at+';font-size:'+(20*S)+'px;color:rgba(245,243,240,.55);opacity:'+(u*(1-grow))+'">'+kinds[i]+'</div>';
    }
    const cap = easeOut(seg(b, 30, 30.8)) * (1 - grow);
    const capAt = C.tall ? 'left:'+(W * .46)+'px;right:'+(48*S)+'px;top:'+(H * .5 - 90*S)+'px;font-size:'+(78*S)+'px' : 'left:0;right:0;top:'+(120*S)+'px;text-align:center;font-size:'+(64*S)+'px';
    out += '<div class="abs" style="'+capAt+';font-weight:700;letter-spacing:-.035em;line-height:1.05;text-wrap:balance;opacity:'+cap+';transform:translateY('+((1-cap)*20)+'px)">Chain anything into anything.</div>';
    if (b >= 33) out += label(C.chainNames.join(' → '), easeOut(seg(b, 33, 34)));
    out += flash(b, 32, .3);
  }

  // ── D2. One source, every effect (36–44): the hero again, a new effect on each beat ─────
  if (b >= 36 && b < 44) {
    const k = Math.floor(b - 36);
    out += full('fx' + k, T);
    out += flash(b, 36 + k, .22);
    const cap = easeOut(seg(b, 36, 36.6));
    out += '<div class="abs" style="left:0;right:0;top:'+(C.tall ? 200*S : 64*S)+'px;display:flex;justify-content:center;opacity:'+cap+';transform:translateY('+((1-cap)*-20)+'px)">'
      + '<div style="padding:'+(18*S)+'px '+(34*S)+'px;border-radius:'+(20*S)+'px;background:rgba(8,8,12,.7);backdrop-filter:blur(20px);font-size:'+(52*S)+'px;font-weight:700;letter-spacing:-.03em;white-space:nowrap">One source. <span style="color:'+ACCENT+'">Every effect.</span></div></div>';
    out += label(C.fxSource + ' → ' + C.fx[k]);
  }

  // ── D3. Feedback (44–60): the loop, then four looks ─────────────────────────────────────
  if (b >= 44 && b < 60) {
    if (b < 48.6) {
      // The loop: the source into Feedback, whose output feeds back into itself.
      out += '<div class="fill" style="background-image:radial-gradient(rgba(255,255,255,.09) 1.6px,transparent 1.6px);background-size:'+(40*S)+'px '+(40*S)+'px"></div>';
      const grow = easeInOut(seg(b, 47.4, 48.6));
      const nw = C.tall ? W * .38 : W * .3, nh = nw * H / W;
      const p0 = C.tall ? { x: W/2 - nw/2, y: 350*S } : { x: W * .12, y: H * .5 - nh/2 + 40*S };
      const p1 = C.tall ? { x: W/2 - nw/2, y: 350*S + nh + 110*S } : { x: W * .88 - nw, y: H * .5 - nh/2 + 40*S };
      const word = 'Feedback'.split('').map((chr, k) => {
        const v = spring(clamp((seg(b, 44, 45) * 8 - k) / 2.5));
        return '<span style="display:inline-block;opacity:'+clamp(v*1.5)+';transform:translateY('+((1-v)*50*S)+'px)">'+chr+'</span>';
      }).join('');
      out += '<div class="abs" style="left:0;right:0;top:'+(C.tall ? 90*S : 70*S)+'px;text-align:center;opacity:'+(1-grow)+'"><div style="font-size:'+(C.tall ? 150 : 140)*S+'px;font-weight:800;letter-spacing:-.05em;line-height:1">'+word+'</div>'
        + '<div style="margin-top:'+(18*S)+'px;font-size:'+(40*S)+'px;font-weight:500;color:rgba(245,243,240,.75);opacity:'+easeOut(seg(b, 45.5, 46.2))+'">Every frame feeds the next.</div></div>';
      // Cables: source → Feedback, and Feedback's loop back into itself.
      const u0 = easeInOut(seg(b, 44.6, 45.4)), u1 = easeInOut(seg(b, 45.6, 46.8));
      let svg = '';
      const sw = 5*S;
      const path = (d, u) => '<path d="'+d+'" fill="none" stroke="'+ACCENT+'" stroke-width="'+sw+'" stroke-linecap="round" pathLength="1" stroke-dasharray="1" stroke-dashoffset="'+(1-u)+'"/>';
      if (C.tall) {
        const a = { x: p0.x + nw/2, y: p0.y + nh }, z = { x: p1.x + nw/2, y: p1.y };
        svg += path('M'+a.x+' '+a.y+' L'+z.x+' '+z.y, u0);
        const r = { x: p1.x + nw, y: p1.y + nh*.7 }, l = { x: p1.x + nw, y: p1.y + nh*.3 }, out2 = 170*S;
        svg += path('M'+r.x+' '+r.y+' C'+(r.x+out2)+' '+r.y+' '+(l.x+out2)+' '+l.y+' '+l.x+' '+l.y, u1);
      } else {
        const a = { x: p0.x + nw, y: p0.y + nh/2 }, z = { x: p1.x, y: p1.y + nh/2 };
        svg += path('M'+a.x+' '+a.y+' C'+(a.x+120*S)+' '+a.y+' '+(z.x-120*S)+' '+z.y+' '+z.x+' '+z.y, u0);
        const r = { x: p1.x + nw*.7, y: p1.y + nh }, l = { x: p1.x + nw*.3, y: p1.y + nh }, down = 150*S;
        svg += path('M'+r.x+' '+r.y+' C'+r.x+' '+(r.y+down)+' '+l.x+' '+(l.y+down)+' '+l.x+' '+l.y, u1);
      }
      out += '<svg class="fill" width="'+W+'" height="'+H+'" style="opacity:'+(1-grow)+'">'+svg+'</svg>';
      out += '<div style="opacity:'+(1-grow)+'">' + node('fb0-n0', T, p0.x, p0.y, nw, nh, C.fbSources[0], seg(b, 44.2, 44.8), '', C.tall) + '</div>';
      out += node('fb0-n1', T, lerp(p1.x, 0, grow), lerp(p1.y, 0, grow), lerp(nw, W, grow), lerp(nh, H, grow), grow < .2 ? 'Feedback' : '', seg(b, 45, 45.6), 'transform:none', C.tall);
      out += flash(b, 48, .3);
    } else {
      const k = b < 51 ? 0 : 1 + Math.floor((b - 51) / 3);
      out += full(k === 0 ? 'fb0-n1' : 'fb' + k, T);
      if (k > 0) out += flash(b, 51 + 3*(k-1), .3);
      out += label('Feedback · ' + C.fbLabels[k] + '<span style="opacity:.5">&nbsp;&nbsp;' + C.fbSources[k] + '</span>');
    }
  }

  { const b = t * 2 - C.later;   // the closing sections keep their own beat numbers
  // ── E. A grid (36–40): one more on each beat, all on the same beat ──────────────────────
  if (b >= 36 && b < 40) {
    const g = 10 * S, cw = (W - g) / 2, ch = (H - g) / 2;
    for (let i = 0; i < 4; i++) {
      const u = seg(b, 36 + i, 36.5 + i);
      if (u <= 0) continue;
      const x = (i % 2) * (cw + g), y = Math.floor(i / 2) * (ch + g);
      const k = spring(u);
      out += '<div class="abs" style="left:'+x+'px;top:'+y+'px;width:'+cw+'px;height:'+ch+'px;overflow:hidden;opacity:'+clamp(u*3)+';transform:scale('+(.9+.1*k)+');border-radius:'+(10*S)+'px">'+full('g'+i, T)+'</div>';
    }
    const cap = easeOut(seg(b, 38, 38.6));
    out += '<div class="abs" style="left:50%;top:50%;transform:translate(-50%,-50%) scale('+(.9+.1*spring(seg(b, 38, 38.6)))+');padding:'+(26*S)+'px '+(44*S)+'px;border-radius:'+(24*S)+'px;background:rgba(8,8,12,.72);backdrop-filter:blur(20px);font-size:'+(60*S)+'px;font-weight:700;letter-spacing:-.035em;white-space:nowrap;opacity:'+cap+'">One beat. Every visual.</div>';
  }

  // ── F. Quick cuts (40–44): every half beat ──────────────────────────────────────────────
  if (b >= 40 && b < 44) {
    const k = Math.floor((b - 40) * 2);
    out += full('q' + k, T);
    out += flash(b, 40 + k/2, .2);
  }

  // ── G. Free. macOS. Open source. (44–48) ────────────────────────────────────────────────
  if (b >= 44 && b < 48) {
    out += full('heroB', T);
    out += '<div class="fill" style="background:radial-gradient(ellipse at center,rgba(8,8,12,.55),rgba(8,8,12,.1) 70%)"></div>';
    const words = ['Free.', 'macOS.', 'Open source.'];
    const size = (C.tall ? 130 : 120) * S;
    let html = '';
    words.forEach((wd, k) => {
      const u = seg(b, 44 + k, 44.5 + k);
      if (u <= 0) return;
      const v = spring(u);
      html += '<div style="opacity:'+clamp(u*2)+';transform:translateY('+((1-v)*size*.4)+'px) scale('+(.92+.08*v)+')">'+wd+'</div>';
    });
    out += '<div class="fill" style="display:flex;flex-direction:column;align-items:center;justify-content:center;gap:'+(10*S)+'px;font-size:'+size+'px;font-weight:750;letter-spacing:-.045em;line-height:1.02;text-shadow:0 6px 40px rgba(0,0,0,.5)">'+html+'</div>';
    out += label(C.heroB);
    out += flash(b, 44, .3);
  }

  // ── H. Last look (48–56): then it folds away into the mark ──────────────────────────────
  if (b >= 48 && b < 56.2) {
    const fold = easeInOut(seg(b, 52, 53.6));
    const sink = easeIn(seg(b, 54.4, 55.9));
    const scale = lerp(1, .56, fold) * lerp(1, .04, sink);
    const radius = lerp(0, 36, fold) * (1 + sink * 6);
    out += '<div class="fill" style="transform:scale('+scale+');opacity:'+(1 - sink*.6)+';border-radius:'+radius+'px;overflow:hidden;box-shadow:0 0 0 '+(1.5*fold)+'px rgba(255,255,255,.18),0 40px 120px rgba(0,0,0,'+(.5*fold)+')">'+full('heroC', T)+'</div>';
    if (b < 52) out += label(C.heroC, easeOut(seg(b, 48.5, 49.5)));
    out += flash(b, 48, .3);
  }

  // ── I. The mark (56–): Nottawa, the line, the address; silence from the track's stop ────
  if (b >= 55.9) {
    const nameU = seg(b, 56, 57.2);
    const letters = 'Nottawa'.split('').map((chr, k) => {
      const v = spring(clamp((nameU * 7 - k) / 2.5));
      return '<span style="display:inline-block;opacity:'+clamp(v*1.5)+';transform:translateY('+((1-v)*60*S)+'px)">'+chr+'</span>';
    }).join('');
    const rule = easeOut(seg(b, 57.2, 58));
    const tag = easeOut(seg(b, 58, 58.8));
    const url = easeOut(seg(b, 59.5, 60.3));
    const glow = easeOut(seg(b, 56, 58));
    out += '<div class="fill" style="background:radial-gradient(ellipse at 50% 46%,rgba(217,101,58,'+(.16*glow)+'),transparent 60%)"></div>';
    out += '<div class="fill" style="display:flex;flex-direction:column;align-items:center;justify-content:center">'
      + '<div style="font-size:'+(C.tall ? 210 : 230)*S+'px;font-weight:800;letter-spacing:-.05em;line-height:1">'+letters+'</div>'
      + '<div style="width:'+(96*S*rule)+'px;height:'+(5*S)+'px;border-radius:3px;background:'+ACCENT+';margin:'+(40*S)+'px 0 '+(34*S)+'px"></div>'
      + '<div style="font-size:'+(54*S)+'px;font-weight:500;letter-spacing:-.02em;color:rgba(245,243,240,.82);opacity:'+tag+';transform:translateY('+((1-tag)*16)+'px)">Live visuals for everyone.</div>'
      + '<div class="mono" style="margin-top:'+(70*S)+'px;font-size:'+(30*S)+'px;color:'+ACCENT+';opacity:'+url+';letter-spacing:.08em;text-transform:none">nottawa.app</div>'
      + '</div>';
    out += '<div class="abs mono" style="left:0;right:0;bottom:'+(60*S)+'px;text-align:center;font-size:'+(18*S)+'px;color:rgba(245,243,240,.45);opacity:'+url+'">Free · macOS · Open source · Every frame rendered by Nottawa</div>';
  }
  }

  out += '<div class="grain" style="background:url('+grain[fi % grain.length]+');transform:translate('+((fi*37)%64)+'px,'+((fi*53)%64)+'px)"></div>';
  root.innerHTML = out;
  await Promise.all(need.map(img));
  await Promise.all([...root.querySelectorAll('img')].map((i) => i.decode().catch(() => {})));
  await document.fonts.ready;
};
</script></body></html>`;
}

async function render(format, footage, outFile, track) {
  const { chromium } = await import('playwright');
  const { W, H } = FORMATS[format];
  const html = page(format, resolve(footage));
  const tmp = resolve(dirname(resolve(outFile)), `_promo-${format}`);
  rmSync(tmp, { recursive: true, force: true });
  mkdirSync(tmp, { recursive: true });
  writeFileSync(resolve(tmp, 'promo.html'), html);
  const browser = await chromium.launch({ channel: 'chrome', headless: true, args: ['--allow-file-access-from-files'] });
  const pg = await (await browser.newContext({ viewport: { width: W, height: H }, deviceScaleFactor: 1 })).newPage();
  await pg.goto(pathToFileURL(resolve(tmp, 'promo.html')).href);
  if (process.env.STILLS) {
    for (const t of process.env.STILLS.split(',').map(Number)) {
      await pg.evaluate((t) => window.seek(t), t);
      await pg.screenshot({ path: resolve(dirname(resolve(outFile)), `still-${format}-${t}.png`) });
    }
    await browser.close();
    rmSync(tmp, { recursive: true, force: true });
    return;
  }
  const n = Math.round(DURATION * FPS);
  for (let i = 0; i < n; i++) {
    await pg.evaluate((t) => window.seek(t), i / FPS);
    await pg.screenshot({ path: resolve(tmp, `${String(i).padStart(5, '0')}.jpg`), type: 'jpeg', quality: 94 });
    if (i % 120 === 0) console.error(`frame ${i}/${n}`);
  }
  await browser.close();
  // The track from START to its stop (a short fade so the cut doesn't click), then silence.
  const len = STOP - START;
  execFileSync('ffmpeg', ['-y', '-loglevel', 'error', '-framerate', String(FPS), '-i', resolve(tmp, '%05d.jpg'),
    '-ss', String(START), '-t', String(len), '-i', track,
    '-filter_complex', `[1:a]afade=t=out:st=${len - 0.25}:d=0.25,apad=whole_dur=${DURATION}[a]`, '-map', '0:v', '-map', '[a]',
    '-c:v', 'libx264', '-preset', 'slow', '-crf', '17', '-pix_fmt', 'yuv420p',
    '-c:a', 'aac', '-b:a', '192k', '-t', String(DURATION), '-movflags', '+faststart', resolve(outFile)]);
  rmSync(tmp, { recursive: true, force: true });
  console.error(`wrote ${outFile} (${DURATION} s)`);
}

const [cmd, format, ...rest] = process.argv.slice(2);
if (!FORMATS[format]) throw new Error('format: wide or tall');
if (cmd === 'plan') process.stdout.write(JSON.stringify(plan(format), null, 1) + '\n');
else if (cmd === 'render') {
  const [footage, outFile, track = resolve(HERE, '../../audio/120A Nottawa Song.mp3')] = rest;
  if (!footage || !outFile || !existsSync(footage)) throw new Error('render <wide|tall> <footageDir> <out.mp4> [track]');
  await render(format, footage, outFile, track);
} else throw new Error('plan | render');
