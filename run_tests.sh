#!/bin/bash

# Create logs directory if it doesn't exist
mkdir -p logs

# Change to the directory containing the processor executable
cd mips_cpu

# Loop through all .o files in the test_data_pipeline/build/ directory
for file in ../test_data_pipeline/bins/*.o; do
    # Check if the file exists
    if [ -f "$file" ]; then
        # Extract the base filename without path and extension
        base_filename=$(basename "$file" .o)
        
        # Run the processor and output to log file
        ./processor --bmk="$file" -O1 > "../logs/${base_filename}.log" 2>&1
        
        # Check the exit status of the processor
        if [ $? -eq 0 ]; then
            echo "Processed $base_filename successfully"
        else
            echo "Error processing $base_filename" >&2
        fi
    fi
done

echo "Testing complete. Check the logs directory for results."
