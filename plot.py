import matplotlib.pyplot as plt

with open('fft.txt', 'r') as file:
    windows = file.readlines()

    plt.ylim(top = 50000)

    for i, window in enumerate(windows):
        line, = plt.plot(eval(window), 'b')
        plt.xlabel(f'window {i}')
        plt.savefig(f'plot{i}.png')
        line.remove()