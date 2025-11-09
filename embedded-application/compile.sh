#!/usr/bin/env bash

# Usage: ./compile.sh [true|false]
# If the first argument is true (default), the script will log performance metrics to perf.log
# If false, it will just compile and run the program (no logging / monitoring).

PARAM=${1:-true}
param_lc=$(echo "$PARAM" | tr '[:upper:]' '[:lower:]')
case "$param_lc" in
	1|true|yes|y)
		LOGGING=1
		;;
	0|false|no|n)
		LOGGING=0
		;;
	*)
		echo "Usage: $0 [true|false]"
		exit 2
		;;
esac

if [ "$LOGGING" -eq 1 ]; then
	# Log file (appended)
	LOG="perf.log"
	# Redirect all stdout/stderr to the log file
	exec >>"$LOG" 2>&1
	echo "========== Run started: $(date +'%Y-%m-%d %H:%M:%S') =========="
fi

# Build
cmake -B build -S . -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build

if [ "$LOGGING" -eq 0 ]; then
	# Just run the program in foreground and exit
	./build/main
	exit $?
fi

# Logging enabled: start the program in background and monitor
./build/main &
PID=$!
echo "Monitoring performance of PID=$PID"

# Sampling parameters
max_samples=100
sample_count=0

# Sums for averaging (floating point via awk)
sum_cpu=0
sum_mem=0
sum_rss_mb=0

# Collect up to max_samples while process is alive
while kill -0 $PID 2>/dev/null && [ "$sample_count" -lt "$max_samples" ]; do
		line="$(ps -p $PID -o %cpu=,%mem=,rss=)"
		read cpu mem rss <<< "$line"
		# convert RSS (KB) to MB with 2 decimals
		mb=$(awk -v r="$rss" 'BEGIN{printf "%.2f", r/1024}')

		# Log a sample line
		printf "%s PID=%s CPU=%s%% MEM=%s%% RSS=%sMB\n" "$(date +'%Y-%m-%d %H:%M:%S')" $PID "$cpu" "$mem" "$mb"

		# Update sums (use awk to handle floating point addition)
		sum_cpu=$(awk -v a="$sum_cpu" -v b="$cpu" 'BEGIN{printf "%.6f", a + b}')
		sum_mem=$(awk -v a="$sum_mem" -v b="$mem" 'BEGIN{printf "%.6f", a + b}')
		sum_rss_mb=$(awk -v a="$sum_rss_mb" -v b="$mb" 'BEGIN{printf "%.6f", a + b}')

		sample_count=$((sample_count + 1))
		sleep 1
done

# If the process exited before we gathered max_samples, note collected samples
if [ "$sample_count" -lt "$max_samples" ]; then
		echo "Process ended or reached sampling stop; collected $sample_count samples"
fi

# Compute and log averages
if [ "$sample_count" -gt 0 ]; then
		avg_cpu=$(awk -v s="$sum_cpu" -v n="$sample_count" 'BEGIN{printf "%.4f", s / n}')
		avg_mem=$(awk -v s="$sum_mem" -v n="$sample_count" 'BEGIN{printf "%.4f", s / n}')
		avg_rss_mb=$(awk -v s="$sum_rss_mb" -v n="$sample_count" 'BEGIN{printf "%.4f", s / n}')

		echo "---- Summary (based on $sample_count samples) ----"
		echo "Average CPU% : $avg_cpu"
		echo "Average MEM% : $avg_mem"
		echo "Average RSS (MB): $avg_rss_mb"
else
		echo "No samples collected; nothing to average."
fi

echo "========== Run finished: $(date +'%Y-%m-%d %H:%M:%S') =========="

exit 0