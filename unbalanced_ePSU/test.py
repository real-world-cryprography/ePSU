import os
import subprocess
import argparse

def run_command(command):
    try:
        subprocess.run(command, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"An error occurred: {e}")

def main():
    parser = argparse.ArgumentParser(description='Script to automate the test process', formatter_class=argparse.RawTextHelpFormatter)
    protocol_group = parser.add_argument_group('Protocol', 'Select one of the following protocols:')
    protocol_group.add_argument('-pecrg', action='store_true', help='Enable the pECRG protocol')
    protocol_group.add_argument('-pnecrg', action='store_true', help='Enable the pnECRG protocol')
    protocol_group.add_argument('-pecrg_necrg_otp', action='store_true', help='Enable the pECRG_nECRG_OTP protocol', default=True)

    parameter_group = parser.add_argument_group('Parameters', 'Configuration for protocol execution:')
    parameter_group.add_argument('-cn', type=int, required=True, help='If the number of elements in each set less than 2^20, set to 1; otherwise, set to 2.')
    parameter_group.add_argument('-nt', type=int, default=1, help='Number of threads, default 1')
    parameter_group.add_argument('-nn', type=int, default=12, help='Logarithm of set size, default 12')

    args = parser.parse_args()

    # Check if at least one protocol is selected, if not, print help and exit
    if not (args.pecrg or args.pnecrg or args.pecrg_necrg_otp):
        parser.print_help()
        print("\nError: You must specify one protocol option (-pecrg, -pnecrg, or -pnecrgotp).")
        exit(1)

    # Determine protocol based on arguments
    protocol_command = ""
    if args.pecrg:
        protocol_command = "-pecrg"
        protocol_name = "test_pecrg"
    elif args.pnecrg:
        protocol_command = "-pnecrg"
        protocol_name = "test_pnecrg"
    elif args.pecrg_necrg_otp:
        protocol_command = "-pecrg_necrg_otp"
        protocol_name = "test_pecrg_necrg_otp"

    # Get the absolute path of the current script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    # Build other paths based on the location of the current script
    mcrg_dir = os.path.join(script_dir, 'MCRG', 'build')
    param_dir = os.path.join(script_dir, 'MCRG', 'parameters')
    pnecrg_OTP_dir = os.path.join(script_dir, 'pECRG_nECRG_OTP', 'build')
    auto_test = os.path.join(script_dir, 'MCRG', 'tools')

    # Copy files
    run_command(f'cp {os.path.join(auto_test, "auto_test.py")} {mcrg_dir}/')
    run_command(f'cp {os.path.join(param_dir, "16M-1024.json")} {mcrg_dir}/')

    # Create a folder if it does not exist
    if not os.path.exists(os.path.join(mcrg_dir, 'randomM')):
        os.makedirs(os.path.join(mcrg_dir, 'randomM'))

    # Run a Python script
    os.chdir(mcrg_dir)
    run_command(f'python3 auto_test.py -nn {args.nn}')

    # Start receiver and sender in the background
    receiver_sender_command = f'{os.path.join(mcrg_dir, "bin", "receiver_cli_ddh")} -d db.csv -p {os.path.join(param_dir, "16M-1024.json")} --port 60000 -t 1 & ' + \
                              f'{os.path.join(mcrg_dir, "bin", "sender_cli_ddh")} -q query.csv --port 60000 -a 127.0.0.1 -f {os.path.join(param_dir, "16M-1024.json")} -t 1'
    
    print("\n\n\nstart for MCRG\n\n\n")
    run_command(receiver_sender_command)
    print("\n\nend for MCRG\n\n\n")
    # Switch to another directory
    os.chdir(pnecrg_OTP_dir)

    # Start two instances of main in the background with the selected protocol
    main_command = f'{os.path.join(pnecrg_OTP_dir, protocol_name)} {protocol_command} -cn {args.cn} -nt {args.nt} -r 0 & ' + \
                   f'{os.path.join(pnecrg_OTP_dir, protocol_name)} {protocol_command} -cn {args.cn} -nt {args.nt} -r 1'
    
    print("\n\nstart for" + f' {protocol_name}' + "\n\n\n")
    run_command(main_command)
    print("\n\nend for" + f' {protocol_name}' + "\n\n\n")
    # Switch back to the original directory
    os.chdir(script_dir)

if __name__ == '__main__':
    main()

