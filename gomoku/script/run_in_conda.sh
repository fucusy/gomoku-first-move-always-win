#!/bin/zsh

# Initialize Conda for the shell script
# source ~/anaconda3/bin/activate   # Adjust path if necessary
# conda init zsh
source /Users/qiang/.zshrc

# Activate the specific Conda environment
conda activate gomoku

#
cd /Users/qiang/Documents/github/gomoku-first-move-always-win/gomoku/script

# Run your Python script
python tornado_server.py 8080

