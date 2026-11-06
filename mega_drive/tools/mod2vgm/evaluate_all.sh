#!/bin/sh
# Score every PC module in parallel: tools/mod2vgm/evaluate_all.sh [secs]
cd "$(dirname "$0")/../.." || exit 1
SECS=${1:-40}
ls data/games/RockDroid1/music/*.mod data/games/RockDroid1/music/*.s3m | \
  xargs -P 8 -I{} python3 tools/mod2vgm/evaluate.py --secs "$SECS" {} | sort | \
  awk '{print} /chroma/{c+=$3;m+=$5;e+=$7;o+=$9;n++} END{printf "MEAN(%d)  chroma %.3f  mel %.3f  env %.3f  onset %.3f\n",n,c/n,m/n,e/n,o/n}'
