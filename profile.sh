#!/bin/bash

# Number of times to run the command
runs=5

# The command to time
cmd="du -sh * | sort -rh"

total_time=0

echo "Running '$cmd' $runs times..."

for i in $(seq 1 $runs)
do
    start=$(date +%s.%N)
    eval $cmd > /dev/null
    end=$(date +%s.%N)

    # Calculate the execution time
    execution_time=$(echo "$end - $start" | bc)
    
    echo "Run $i: $execution_time seconds"
    
    total_time=$(echo "$total_time + $execution_time" | bc)
done

# Calculate average time
average_time=$(echo "scale=4; $total_time / $runs" | bc)

echo "Average execution time: $average_time seconds"
