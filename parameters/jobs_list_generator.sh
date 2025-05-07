#!/bin/bash

# Parameters
SPIN_DIMENSIONS="2"
SPATIAL_DIMENSIONS="3"
LENGTH="24"
SWEEPS_THERMALIZATION="10000"
SWEEPS_SIMULATION="100000"
SKIP="10"
BATCH="0"
SAVE_HISTORY="0"
# BETA="0.05 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.6 0.65 0.7 0.75 0.8 0.85 0.9 0.95 1.0 1.05 1.1 1.15 1.2 1.25 1.3 1.35 1.4 1.45 1.5 1.55 1.6 1.65 1.7 1.75 1.8 1.85 1.9 1.95 2.05 2.1 2.15 2.2 2.25 2.3 2.35 2.4 2.45 2.5 2.55 2.6"

START=0.451
END=0.4557
STEP=0.0001

PARAM_FILE="./jobs_list.txt"

# Saves current jobs_list.txt file
if [ -f "$PARAM_FILE" ]; then
    # Get date YYYYMMDD_HHMMSS
    TIMESTAMP=$(date +%Y%m%d_%H%M%S)
    mv "$PARAM_FILE" "./jobs_list_${TIMESTAMP}.txt"
    echo "The file jobs_list.txt was renamed as jobs_list_${TIMESTAMP}.txt"
fi

echo "Generating parameters file..."

{
    for val in $(seq $START $STEP $END); do
        echo "$SPIN_DIMENSIONS $SPATIAL_DIMENSIONS $LENGTH $val $SWEEPS_THERMALIZATION $SWEEPS_SIMULATION $SKIP $BATCH $SAVE_HISTORY"
    done
} > "$PARAM_FILE"

echo "The file jobs_list.txt was successfully generated"
