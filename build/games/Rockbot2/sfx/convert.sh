#!/usr/bin/env bash

set -x

### Convert WAV bitratio to 16k and depth to 8
for filename in ./*; do
    basename=${filename##*/}
    if [[ $basename =~ ".wav" ]]; then
        sox "$filename" -b 8 -r 16000 "./compressed/$filename"
    fi
done
