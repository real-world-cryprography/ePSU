#!/bin/bash

# Default configuration
selected_protocol="pecrg_necrg_otp"  # Default protocol
nt_value=4                           # Default nt value
nn_start=10                          # Default 2^10
nn_end=20                            # Default 2^20
nn_step=2                            # Default step 2

# Valid protocol list
valid_protocols=("pecrg_necrg_otp" "pecrg" "pnecrg")

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -start)
            nn_start="$2"
            shift 2
            ;;
        -end)
            nn_end="$2"
            shift 2
            ;;
        -step)
            nn_step="$2"
            shift 2
            ;;
        -nt)
            nt_value="$2"
            shift 2
            ;;
        -protocol)
            selected_protocol="$2"
            shift 2
            ;;
        *)
            echo "Error: Unknown option $1"
            exit 1
            ;;
    esac
done

# Validate input
validate_number() {
    [[ "$1" =~ ^[0-9]+$ ]] || {
        echo "Invalid value: $1 must be a positive integer"
        exit 1
    }
}

# Protocol validation
if [[ ! " ${valid_protocols[@]} " =~ " ${selected_protocol} " ]]; then
    echo "Error: Invalid protocol '$selected_protocol'"
    echo "Valid options:"
    printf "• %s\n" "${valid_protocols[@]}"
    exit 1
fi

validate_number "$nn_start"
validate_number "$nn_end"
validate_number "$nn_step"
validate_number "$nt_value"

# Generate nn sequence
generate_nn_sequence() {
    seq "$nn_start" "$nn_step" "$nn_end"
}

# Get protocol flag
case "$selected_protocol" in
    "pecrg_necrg_otp") flag="-pecrg_necrg_otp" ;;
    "pecrg") flag="-pecrg" ;;
    "pnecrg") flag="-pnecrg" ;;
esac

# Print configuration
echo "=== Test Configuration ==="
echo "Protocol        : $selected_protocol ($flag)"
echo "nt value        : $nt_value"
echo "nn range        : 2^$nn_start to 2^$nn_end (step $nn_step)"
echo "Total test cases: $(generate_nn_sequence | wc -w)"
echo "=========================="
echo

# Run tests
for nn in $(generate_nn_sequence); do
    echo "=== Running with nn=$nn ==="
    python3 test.py $flag -cn 1 -nt "$nt_value" -nn "$nn"
    echo "=== Completed nn=$nn ==="
    echo
done

echo "All tests completed!"
