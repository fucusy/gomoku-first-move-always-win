import sys
import os

if __name__ == '__main__':
    input = sys.argv[1]
    content = []
    if os.path.isfile(input):
        for line in open(input, 'r'):
            content.append(line.strip("\n"))
    else:
        content = input.strip("_").split("_")

    filename = "./tmp.txt"
    with open(filename, 'w') as f:
        for c in content:
            f.write(c + "\n")
