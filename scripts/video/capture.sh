#!/bin/zsh
# Renders a video plan's shots in NottawaApp (the "video" self-test scenario):
# each shot's chain, frame by frame, reacting to the track as if it played.
#
#   scripts/video/capture.sh <plan.json> <outDir> [track.mp3]
#
# The plan names its shots (see VideoPlan in NottawaApp/Engine/SelfTest.swift);
# its "audio" is filled in here. Each shot lands in <outDir>/<shot>/ as JPEG
# frames (00000.jpg, ...) and shot.json, plus <shot>.mp4 for a look, and
# <outDir>/report.txt.
# Runs in a throwaway session: your graph is never touched. Build NottawaApp
# (Debug) first.
set -eu
cd "$(dirname "$0")/../.."
PLAN=${1:?plan.json}
OUT=${2:?outDir}
TRACK=${3:-"audio/120A Nottawa Song.mp3"}

CONTAINER=~/Library/Containers/com.joecrozier.nottawa/Data
WORK=$CONTAINER/tmp/video-capture  # (the sandboxed app can't reach the repo)
APPDATA=$CONTAINER/tmp/video-appdata
rm -rf "$WORK" "$APPDATA" "$CONTAINER/tmp/selftest"
mkdir -p "$WORK/out" "$APPDATA" "$OUT"

ffmpeg -v error -y -i "$TRACK" -ac 1 -ar 48000 -f f32le "$WORK/track.f32"
python3 -c 'import json,sys; p=json.load(open(sys.argv[1])); p["audio"]=sys.argv[2]; p["sampleRate"]=48000; json.dump(p,open(sys.argv[3],"w"))' \
  "$PLAN" "$WORK/track.f32" "$WORK/plan.json"

pkill -x NottawaApp 2>/dev/null || true
open -n -W -g --env NOTTAWA_SELFTEST=video --env NOTTAWA_APPDATA="$APPDATA" \
  --env NOTTAWA_VIDEO_PLAN="$WORK/plan.json" --env NOTTAWA_VIDEO_OUT="$WORK/out" \
  bin/NottawaApp.app --args -ApplePersistenceIgnoreState YES
cp "$CONTAINER/tmp/selftest/report.txt" "$OUT/report.txt"
grep -E "video|FAIL" "$OUT/report.txt"

for dir in "$WORK"/out/*/; do
  name=$(basename "$dir")
  rm -rf "$OUT/$name"; mv "$dir" "$OUT/$name"
  fps=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["fps"])' "$OUT/$name/shot.json")
  ffmpeg -v error -y -framerate "$fps" -i "$OUT/$name/%05d.jpg" -c:v libx264 -crf 18 -pix_fmt yuv420p "$OUT/$name.mp4"
done
rm -rf "$WORK" "$APPDATA"
