#!/bin/bash
# Runs NottawaApp's built-in self-test (NottawaApp/Engine/SelfTest.swift)
# and scores it. Needs a built bin/NottawaApp.app and no running copy.
#
#   scripts/mac_selftest.sh [scenarios]
#     default: files,system,tap,tapauto,persist,settings,settingspersist,settingsui,
#              audiosettings,audiolaunch,cachesettings,outputwindows,outputwindowslaunch,
#              diagnostics,diagnosticslaunch,texturelock,golden,workflows
#
# "golden" and "workflows" (docs/TESTING.md) launch on their own with a
# throwaway session (NOTTAWA_APPDATA), so your graph is never touched.
# Golden images: Tests/golden/macos. NOTTAWA_GOLDEN=record renders them
# again (then look at the changes in git); =stabilize moves entries that
# differ from their image into unstable.txt.
#
# "persist" and "settingspersist" launch twice: change a parameter (or some
# settings) and quit at once, then check on relaunch. settingsui leaves
# images of the Settings window (ui-*.png) in the output folder. Audio scenarios are scored by scripts/mac_selftest_bpm.py.
# "texturelock" launches on its own with scripts/mac_gpu_hog.swift loading the
# GPU (NOTTAWA_SELFTEST_LOCK_SECONDS, default 15).
# NOTTAWA_SELFTEST_SECONDS (default 40) is the listening time per track.
# NOTTAWA_SELFTEST_CALIB=<wav from Tests/bpm/calib_drums.py> makes "system"
# play that instead, scored against its exact beats.
set -u
cd "$(dirname "$0")/.."

APP=bin/NottawaApp.app/Contents/MacOS/NottawaApp
CONTAINER=~/Library/Containers/com.joecrozier.nottawa/Data
GOLDEN=$CONTAINER/tmp/selftest-golden  # (the sandboxed app can't reach the repo)
APPDATA=
per=${NOTTAWA_SELFTEST_SECONDS:-40}
lock=${NOTTAWA_SELFTEST_LOCK_SECONDS:-15}
OUT=~/Library/Containers/com.joecrozier.nottawa/Data/tmp/selftest
SCENARIOS=${1:-files,system,tap,tapauto,persist,settings,settingspersist,settingsui,audiosettings,audiolaunch,cachesettings,outputwindows,outputwindowslaunch,diagnostics,diagnosticslaunch,texturelock,golden,workflows}

if pgrep -xq NottawaApp; then echo "NottawaApp is running; quit it first" >&2; exit 2; fi
[ -x "$APP" ] || { echo "no $APP; build the NottawaApp scheme" >&2; exit 2; }

# One launch, killed if it hasn't quit by itself in $2 seconds.
launch() {
  rm -f "$OUT/report.txt"
  # Through LaunchServices, ignoring saved window state: the engine starts
  # (and the quit-time save is hooked) from the main window, which a bare
  # launch or a no-window saved state never shows.
  open -n -W -g --env NOTTAWA_SELFTEST="$1" --env NOTTAWA_SELFTEST_SECONDS="$per" --env NOTTAWA_SELFTEST_CLIP="$CLIP" \
    --env NOTTAWA_SELFTEST_CALIB="${NOTTAWA_SELFTEST_CALIB:-}" \
    --env NOTTAWA_SELFTEST_OPENFOLDERS="${NOTTAWA_SELFTEST_OPENFOLDERS:-}" \
    --env NOTTAWA_SELFTEST_LOCK_SECONDS="$lock" \
    ${APPDATA:+--env NOTTAWA_APPDATA="$APPDATA"} \
    --env NOTTAWA_GOLDEN="${NOTTAWA_GOLDEN:-}" --env NOTTAWA_GOLDEN_DIR="$GOLDEN" \
    bin/NottawaApp.app --args -ApplePersistenceIgnoreState YES &
  local opener=$! waited=0
  while kill -0 $opener 2>/dev/null; do
    sleep 1; waited=$((waited + 1))
    if [ $waited -ge "$2" ]; then
      echo "FAIL: $1 did not finish in $2 s; killed" | tee -a "$OUT/report.txt"
      pkill -9 -x NottawaApp
    fi
  done
  grep -qE "^(done|(persist|settings)-set .*quitting|(audiolaunch|outputwindows)-set: .*quitting|diagnostics-set: exiting without quitting)$" "$OUT/report.txt" 2>/dev/null ||
    echo "FAIL: $1 quit without finishing (see Console, process NottawaApp)" | tee -a "$OUT/report.txt"
  cat "$OUT/report.txt" >>"$OUT/all.txt" 2>/dev/null
}

rm -rf "$OUT"; mkdir -p "$OUT"
# A 10 s 720p30 test clip inside the sandbox container (cachesettings): ~1.1 GB to cache.
CLIP=~/Library/Containers/com.joecrozier.nottawa/Data/tmp/selftest-media/clip720p30.mp4
if [ ! -f "$CLIP" ] && echo "$SCENARIOS" | tr ',' '\n' | grep -qx cachesettings; then
  mkdir -p "$(dirname "$CLIP")"
  ffmpeg -v quiet -f lavfi -i testsrc2=size=1280x720:rate=30 -t 10 -c:v libx264 -pix_fmt yuv420p "$CLIP"
fi
in_app=$(echo "$SCENARIOS" | tr ',' '\n' | grep -vE '^(persist|settingspersist|audiolaunch|outputwindowslaunch|diagnosticslaunch|texturelock|golden|workflows)$' | paste -sd, -)
if [ -n "$in_app" ]; then launch "$in_app" $((per * 20 + 120)); fi
if echo "$SCENARIOS" | tr ',' '\n' | grep -qx persist; then
  launch persist-set 60
  launch persist-check 60
fi
if echo "$SCENARIOS" | tr ',' '\n' | grep -qx audiolaunch; then
  launch audiolaunch-set 60
  launch audiolaunch-check 60
fi
if echo "$SCENARIOS" | tr ',' '\n' | grep -qx outputwindowslaunch; then
  launch outputwindows-set 60
  launch outputwindows-check 60
fi
if echo "$SCENARIOS" | tr ',' '\n' | grep -qx diagnosticslaunch; then
  launch diagnostics-set 60
  launch diagnostics-check 60
fi
if echo "$SCENARIOS" | tr ',' '\n' | grep -qx settingspersist; then
  launch settings-set 60
  launch settings-check 60
fi
if echo "$SCENARIOS" | tr ',' '\n' | grep -qx texturelock; then
  hog=$(mktemp -d)/gpuhog
  swiftc -O scripts/mac_gpu_hog.swift -o "$hog" 2>/dev/null || echo "FAIL: texturelock: GPU hog didn't build" >>"$OUT/all.txt"
  [ -x "$hog" ] && "$hog" $((lock + 30)) &
  launch texturelock $((lock + 90))
  pkill -f "$hog" 2>/dev/null; rm -rf "$(dirname "$hog")"
fi

# Rendered pixels, in a throwaway session.
pixels=$(echo "$SCENARIOS" | tr ',' '\n' | grep -xE 'golden|workflows' | paste -sd, -)
if [ -n "$pixels" ]; then
  rm -rf "$GOLDEN" "$CONTAINER/tmp/selftest-appdata"
  mkdir -p "$GOLDEN" Tests/golden/macos
  cp Tests/golden/macos/* "$GOLDEN"/ 2>/dev/null
  APPDATA=$CONTAINER/tmp/selftest-appdata
  launch "$pixels" 900
  APPDATA=
  if [ -n "${NOTTAWA_GOLDEN:-}" ]; then
    rm -f Tests/golden/macos/*.png
    cp "$GOLDEN"/*.png "$GOLDEN"/unstable.txt Tests/golden/macos/
    echo "golden images ($NOTTAWA_GOLDEN): Tests/golden/macos; check the changes in git"
  fi
  rm -rf "$CONTAINER/tmp/selftest-appdata"
fi

echo "== report"; cat "$OUT/all.txt"
if ls "$OUT"/*.csv >/dev/null 2>&1; then
  echo "== beat scoring"
  python3 scripts/mac_selftest_bpm.py "$OUT" bin/NottawaApp.app/Contents/Resources/data/audio
fi
! grep -q FAIL "$OUT/all.txt"
