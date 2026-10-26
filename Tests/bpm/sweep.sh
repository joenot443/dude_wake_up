#!/bin/bash
# Runs bpm_eval (new tracker) over Ballroom once per "NAME=VALUE ..." setting,
# in parallel, then prints each overall line and right-tempo phase summary.
# (WSLENV passes the BT_* settings through to the Windows program.)
cd "$(dirname "$0")"
export WSLENV=BT_MEM:BT_GAIN:BT_EVERY:BT_SNAP
i=0
for setting in "$@"; do
  i=$((i+1))
  (env $setting ./build/Release/bpm_eval.exe --algo new --ballroom ../../.cache/datasets --csv sweep-$i.csv 2>&1 | grep "^ballroom  " > sweep-$i.txt) &
done
wait
i=0
for setting in "$@"; do
  i=$((i+1))
  sum=$(python3 -c "
import csv, statistics as st
rows=[r for r in csv.DictReader(open('sweep-$i.csv')) if r['fin1']=='1']
print(f'rightF {st.mean(float(r[\"beatF\"]) for r in rows):.3f} off {sum(float(r[\"offbeat\"])>0.5 for r in rows)} lowF {sum(float(r[\"beatF\"])<0.5 for r in rows)}')")
  printf "%-44s %s | %s\n" "$setting" "$(awk '{print "Acc1",$3,"Acc2",$4,"exact",$5,"beatF",$11}' sweep-$i.txt)" "$sum"
done
