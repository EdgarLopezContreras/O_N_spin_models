#!/bin/bash

EXECUTABLE="./main"

LOG_DIR="../logs"
mkdir -p "$LOG_DIR"

export EXECUTABLE
export LOG_DIR

# Function to execute job with the right parameters
run_simulation() {
    # Gets parameters from parallel
    PARAMS="$@"

    # Log files name format
    TIMESTAMP=$(date +%Y%m%d_%H%M%S)
    LOG_FILE="$LOG_DIR/log_${TIMESTAMP}_$(echo "$PARAMS" | tr ' ' '_').txt"
    ERR_FILE="$LOG_DIR/error_${TIMESTAMP}_$(echo "$PARAMS" | tr ' ' '_').txt"

    cd $(dirname "$EXECUTABLE")

    echo "Executing: $EXECUTABLE $PARAMS"
    # Executes job and saves logs
    $EXECUTABLE $PARAMS > "$LOG_FILE" 2> "$ERR_FILE"

    if [ $? -ne 0 ]; then
        echo "Error: $EXECUTABLE $PARAMS failed."
    fi


    # Deletes error file if empty
    if [ ! -s "$ERR_FILE" ]; then
        rm "$ERR_FILE"
    fi
}

export -f run_simulation  # exports funtion to use with parallel

# Executes the list of jobs using GNU Parallel
# parallel -j $(nproc) run_simulation ::: $(cat ../parameters/jobs_list.txt)
parallel -j $(nproc) run_simulation < ../parameters/jobs_list.txt