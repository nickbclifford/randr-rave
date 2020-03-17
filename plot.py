import re
import os
import matplotlib.pyplot as plt

line_match = re.compile(r'\[window (?:\d+), (\d+) ms\] total bass: ([\d.]+)')

with open('bass.txt', 'r') as file:
    # plt.ylim(top = 25000)

    for line in file:
        matches = line_match.match(line)
        plt.plot(int(matches[1]), float(matches[2]), 'ro')
    
    plt.xlabel('time since program start (ms)')
    plt.ylabel('total bass intensity')

    plt.show()