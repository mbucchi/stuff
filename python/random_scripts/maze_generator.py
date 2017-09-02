__author__ = 'mabucchi'

import numpy
import random as rd
import matplotlib.pyplot as pyplot

def generate_maze(width=81, height=51, complexity=.75, density=.75):

    shape = ((height // 2) * 2 + 1, (width // 2) * 2 + 1)
    complexity = int(complexity * (5 * (shape[0] + shape[1])))
    density = int(density * (shape[0] // 2 * shape[1] // 2))

    maze = numpy.zeros(shape, dtype=bool)

    maze[0, :] = maze[-1, :] = 1
    maze[:, 0] = maze[:, -1] = 1

    for i in range(density):
        x, y = rd.randint(0, shape[1] // 2) * 2, rd.randint(0, shape[0] // 2) * 2
        maze[y, x] = 1
        for j in range(complexity):
            neighbours = []
            if x > 1:
                neighbours.append((y, x - 2))
            if x < shape[1] - 2:
                neighbours.append((y, x + 2))
            if y > 1:
                neighbours.append((y - 2, x))
            if y < shape[0] - 2:
                neighbours.append((y + 2, x))
            if len(neighbours):
                y_, x_ = rd.choice(neighbours)
                if maze[y_, x_] == 0:
                    maze[y_, x_] = 1
                    maze[y_ + (y - y_) // 2, x_ + (x - x_) // 2] = 1
                    x, y = x_, y_

    pyplot.figure(figsize=(10, 5))
    pyplot.imshow(maze, cmap=pyplot.cm.binary, interpolation='nearest')
    pyplot.xticks([]), pyplot.yticks([])
    pyplot.show()

if __name__ == "__main__":
    generate_maze()
