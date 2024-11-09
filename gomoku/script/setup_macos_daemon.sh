#!/bin/bash

# make sure to add all necessary permisson in macOS, macOS has implemented enhanced privacy protections that require applications to obtain explicit user consent before accessing certain files and folders, such as those in the Desktop, Documents, Downloads, iCloud Drive, and network volumes. This security measure ensures that users have control over their data and are aware of which applications can access sensitive information
# Variables
PYTHONPATH="/Users/qiang/Documents/github/gomoku-first-move-always-win/gomoku/script"   
SCRIPT_PATH="$PYTHONPATH/run_in_conda.sh" # Replace with the path to your Python script
DAEMON_NAME="com.hula.gomoku_backend"   # A unique name for the daemon
PLIST_PATH="/Library/LaunchDaemons/${DAEMON_NAME}.plist"
PORT=8080

# Check if the script exists
if [ ! -f "$SCRIPT_PATH" ]; then
    echo "Error: Python script not found at $SCRIPT_PATH"
    exit 1
fi

# Create the .plist file
echo "Creating $PLIST_PATH..."
sudo cat <<EOL | sudo tee "$PLIST_PATH" > /dev/null
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>$DAEMON_NAME</string>

    <key>ProgramArguments</key>
    <array>
        <string>$SCRIPT_PATH</string>
    </array>

    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>

    <!-- Log files for standard output and errors -->
    <key>StandardOutPath</key>
    <string>$PYTHONPATH/daemon_output.log</string>
    <key>StandardErrorPath</key>
    <string>$PYTHONPATH/daemon_error.log</string>
</dict>
</plist>
EOL

# Set permissions for the .plist file
echo "Setting permissions for $PLIST_PATH..."
sudo chown root:wheel "$PLIST_PATH"
sudo chmod 644 "$PLIST_PATH"

# Load the daemon
echo "Loading the daemon..."
sudo launchctl load "$PLIST_PATH"

echo "Setup complete. The daemon will run the Python script at startup."

