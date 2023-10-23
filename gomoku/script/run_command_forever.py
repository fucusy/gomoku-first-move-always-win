import subprocess
import time
import sys


def run_command(command):
    while True:
        try:
            # Run the command
            result = subprocess.run(command, shell=True, check=True)

            # If the command succeeds, break the loop
            if result.returncode == 0:
                print("Command succeeded!")
                break

        except subprocess.CalledProcessError as e:
            # If the command fails, print an error message and retry
            print(f"Command failed with error: {e}. Retrying in 5 seconds...")
            time.sleep(5)


if __name__ == '__main__':
    # Replace 'your_command_here' with the actual command you want to run
    your_command_here = sys.argv[1]
    print(your_command_here)
    run_command(your_command_here)