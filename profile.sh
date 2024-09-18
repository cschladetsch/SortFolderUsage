#!/bin/bash

# Number of times to run each command
runs=5

# Top-level variables for the two commands to compare
cmd_fast_du_sort="du_sort ." # Replace this with the actual fast_du_sort command or path
cmd_du_sort="du -sh * | sort -rh" # Standard du command

# Initialize time accumulators for both commands
fast_du_sort_total_time=0
du_sort_total_time=0

# Function to calculate average
calculate_average() {
    local total=$1
    local count=$2
    echo "scale=3; $total / $count" | bc
}

# Run both commands $runs times
for i in $(seq 1 $runs); do
    echo "Run #$i:"

    # Time for fast_du_sort (run the first command)
    start_time=$(date +%s.%N)
    fast_du_sort_output=$($cmd_fast_du_sort)  # Run the fast_du_sort command
    end_time=$(date +%s.%N)
    fast_du_sort_time=$(echo "$end_time - $start_time" | bc)
    fast_du_sort_total_time=$(echo "$fast_du_sort_total_time + $fast_du_sort_time" | bc)
    echo "fast_du_sort run #$i time: $fast_du_sort_time seconds"

    # Time for du -sh | sort -rh (run the second command)
    start_time=$(date +%s.%N)
    du_sort_output=$(eval $cmd_du_sort)  # Use eval to run du -sh * | sort -rh
    end_time=$(date +%s.%N)
    du_sort_time=$(echo "$end_time - $start_time" | bc)
    du_sort_total_time=$(echo "$du_sort_total_time + $du_sort_time" | bc)
    echo "du -sh | sort -rh run #$i time: $du_sort_time seconds"

    # Show progressive average times
    fast_du_sort_avg=$(calculate_average "$fast_du_sort_total_time" "$i")
    du_sort_avg=$(calculate_average "$du_sort_total_time" "$i")

    echo "Progressive average time after $i runs:"
    echo "fast_du_sort average: $fast_du_sort_avg seconds"
    echo "du -sh | sort -rh average: $du_sort_avg seconds"
    echo "-------------------------"
done

# Final averages
echo "Final Results after $runs runs:"
fast_du_sort_avg=$(calculate_average "$fast_du_sort_total_time" "$runs")
du_sort_avg=$(calculate_average "$du_sort_total_time" "$runs")
echo "fast_du_sort average: $fast_du_sort_avg seconds"
echo "du -sh | sort -rh average: $du_sort_avg seconds"

