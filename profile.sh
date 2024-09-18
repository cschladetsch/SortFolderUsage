#!/bin/bash

# Number of times to run each command
runs=3

# Top-level variables for the three commands to compare
cmd_du_sort_win="du_sortw.exe ." # Replace this with the actual Windows du_sort executable path
cmd_du_sort_unix="du_sort ."    # Replace this with the actual Linux/WSL du_sort path
cmd_du_sort="du -sh * | sort -rh"     # Standard du command

# Initialize time accumulators for all commands
du_sort_win_total_time=0
du_sort_unix_total_time=0
du_sort_total_time=0

# Function to calculate average
calculate_average() {
    local total=$1
    local count=$2
    echo "scale=3; $total / $count" | bc
}

# Run all commands $runs times
for i in $(seq 1 $runs); do
    echo "Run #$i:"

    # Time for du_sort_win (Windows executable)
    start_time=$(date +%s.%N)
    du_sort_win_output=$($cmd_du_sort_win)  # Run the Windows du_sort command
    end_time=$(date +%s.%N)
    du_sort_win_time=$(echo "$end_time - $start_time" | bc)
    du_sort_win_total_time=$(echo "$du_sort_win_total_time + $du_sort_win_time" | bc)
    echo "du_sort.exe run #$i time: $du_sort_win_time seconds"

    # Time for du_sort_unix (Linux/WSL executable)
    start_time=$(date +%s.%N)
    du_sort_unix_output=$($cmd_du_sort_unix)  # Run the Linux/WSL du_sort command
    end_time=$(date +%s.%N)
    du_sort_unix_time=$(echo "$end_time - $start_time" | bc)
    du_sort_unix_total_time=$(echo "$du_sort_unix_total_time + $du_sort_unix_time" | bc)
    echo "du_sort (Linux/WSL) run #$i time: $du_sort_unix_time seconds"

    # Time for du -sh | sort -rh (Unix command)
    start_time=$(date +%s.%N)
    du_sort_output=$(du -sh * | sort -rh)  # No need for eval, just run the command directly
    end_time=$(date +%s.%N)
    du_sort_time=$(echo "$end_time - $start_time" | bc)
    du_sort_total_time=$(echo "$du_sort_total_time + $du_sort_time" | bc)
    echo "du -sh | sort -rh run #$i time: $du_sort_time seconds"

    # Show progressive average times
    du_sort_win_avg=$(calculate_average "$du_sort_win_total_time" "$i")
    du_sort_unix_avg=$(calculate_average "$du_sort_unix_total_time" "$i")
    du_sort_avg=$(calculate_average "$du_sort_total_time" "$i")

    echo "Progressive average time after $i runs:"
    echo "du_sort.exe average: $du_sort_win_avg seconds"
    echo "du_sort (Linux/WSL) average: $du_sort_unix_avg seconds"
    echo "du -sh | sort -rh average: $du_sort_avg seconds"
    echo "-------------------------"
done

# Final averages
echo "==========================="
echo "Final Results after $runs runs:"
du_sort_win_avg=$(calculate_average "$du_sort_win_total_time" "$runs")
du_sort_unix_avg=$(calculate_average "$du_sort_unix_total_time" "$runs")
du_sort_avg=$(calculate_average "$du_sort_total_time" "$runs")
echo "du_sort.exe average: $du_sort_win_avg seconds"
echo "du_sort (Linux/WSL) average: $du_sort_unix_avg seconds"
echo "du -sh | sort -rh average: $du_sort_avg seconds"

