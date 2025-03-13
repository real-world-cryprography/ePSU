#!/bin/bash

# Default Configuration
PROTOCOL="psu"            # Default protocol
NT_VALUE=4                  # Default nt value
NN_START=10                 # Default start exponent
NN_END=20                   # Default end exponent
NN_STEP=2                   # Default step size
PARTICIPANTS=2              # Fixed number of participants

# Protocol Configuration
declare -A PROTOCOL_MAP=(
    ["pecrg"]="test_pecrg"
    ["pmcrg"]="test_pmcrg"
    ["necrg"]="test_necrg"
    ["pnmcrg"]="test_pnmcrg"
    ["psu"]="test_balanced_epsu"
)

# Parse Command-line Arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -protocol)
            PROTOCOL="$2"
            shift 2
            ;;
        -nt)
            NT_VALUE="$2"
            shift 2
            ;;
        -start)
            NN_START="$2"
            shift 2
            ;;
        -end)
            NN_END="$2"
            shift 2
            ;;
        -step)
            NN_STEP="$2"
            shift 2
            ;;
        *)
            echo "Error: Unknown argument $1"
            exit 1
            ;;
    esac
done

# Validation Functions
validate_number() {
    [[ "$1" =~ ^[0-9]+$ ]] || {
        echo "Error: $2 must be a positive integer"
        exit 1
    }
}

# Input Validation
validate_number "$NN_START" "Start exponent"
validate_number "$NN_END" "End exponent"
validate_number "$NN_STEP" "Step size"
validate_number "$NT_VALUE" "NT value"

if [[ $NN_START -gt $NN_END ]]; then
    echo "Error: Start exponent cannot be greater than end exponent"
    exit 1
fi

# Protocol Validation
if [[ -z "${PROTOCOL_MAP[$PROTOCOL]}" ]]; then
    echo "Error: Invalid protocol '$PROTOCOL'"
    echo "Available protocols:"
    printf "• %s\n" "${!PROTOCOL_MAP[@]}"
    exit 1
fi

# Executable Check
EXECUTABLE="${PROTOCOL_MAP[$PROTOCOL]}"
if [[ ! -x "$EXECUTABLE" ]]; then
    echo "Error: Missing executable $EXECUTABLE"
    echo "Please verify the following files exist and are executable:"
    find . -maxdepth 1 -type f -name "test_*"
    exit 1
fi

# Generate NN Sequence
generate_nn_sequence() {
    seq "$NN_START" "$NN_STEP" "$NN_END"
}

# Print Configuration
echo "=== Test Configuration ==="
echo "Protocol        : $PROTOCOL ($EXECUTABLE)"
echo "NT Value        : $NT_VALUE"
echo "Dataset Range   : 2^$NN_START to 2^$NN_END (step $NN_STEP)"
echo "Participants    : $PARTICIPANTS"
echo "Total Test Cases: $(generate_nn_sequence | wc -w)"
echo "=========================="
echo

# Execute Tests
for nn in $(generate_nn_sequence); do
    echo "▶ Starting test: nn=$nn"
    
    # Launch Participants
    pids=()
    for ((r=0; r<PARTICIPANTS; r++)); do
        echo "   Launching participant -r $r"
        "$EXECUTABLE" -nn "$nn" -nt "$NT_VALUE" -r "$r" &
        pids+=($!)
    done
    
    # Wait for completion
    echo "   Waiting for processes..."
    for pid in "${pids[@]}"; do
        wait "$pid" || {
            echo "Error: Process $pid failed"
            exit 1
        }
    done
    
    echo "✓ Completed test: nn=$nn"
    echo
done

echo "All tests executed successfully!"
