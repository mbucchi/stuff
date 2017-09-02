__author__ = 'mabucchi'

import numpy as np
from decimal import Decimal
from collections import defaultdict


class Graph:

    def __init__(self, nodes):
        self.nodes = dict((n, v) for v, n in enumerate(nodes))
        self.edges = np.ndarray(shape=(len(nodes), len(nodes)), dtype=tuple)
        self.edges.fill((Decimal('inf'), Decimal('inf')))
        self.node_info = dict((n, (Decimal('inf'), 0)) for n in nodes)

    def __repr__(self):
    	return "G(\n  V={},\n  E={}\n)".format(self.nodes, self.edges)

    def add_edge(self, i, j, time, wait):
        i = self.nodes[i]
        j = self.nodes[j]
        self.edges[i, j] = (Decimal(time), Decimal(wait))

    def arc_time(self, i, j):
        i = self.nodes[i]
        j = self.nodes[j]
        return self.edges[i, j][0]

    def arc_freq(self, i, j):
        i = self.nodes[i]
        j = self.nodes[j]
        w = self.edges[i, j][1]

        if w == Decimal('inf'):
            return Decimal(0)

        if w == 0:
            return Decimal('inf')

        return 1 / w

    def mu(self, i):
        return self.node_info[i][0]

    def node_freq(self, i):
        return self.node_info[i][1]

    def mu_freq(self, i):
        time, freq = self.node_info[i]

        if time == Decimal('inf') and freq == 0:
            return Decimal(1)

        return time * freq

    def set_node_info(self, node, info):
        self.node_info[node] = info

    def travel_time(self, arc):
        i, j = arc
        return self.mu(j) + self.arc_time(i, j)

    def spiess(self, end):
        self.set_node_info(end, (Decimal(0), Decimal(0)))
        useful = set()
        to_visit = set((i, j) for i, ii in self.nodes.items() for j, jj in self.nodes.items()
                       if self.edges[ii, jj] != (Decimal('inf'), Decimal('inf')))

        it = 0

        while to_visit:

            it += 1

            current = sorted(to_visit, key=self.travel_time).pop(0)
            to_visit.remove(current)

            i, j = current
            print(
                'iteración {}: se selecciona el arco ({}, {}).'.format(it, i, j), end=' ')

            if self.travel_time(current) > self.mu(i):
                print('Se descarta el arco.')
                continue

            useful.add(current)

            if self.arc_freq(i, j) == Decimal('inf'):
                new_mu = self.mu(j) + self.arc_time(i, j)

            else:
                new_mu = (self.mu_freq(i) + self.arc_freq(i, j) * (self.mu(j) +
                                                                   self.arc_time(i, j))) / (self.node_freq(i) + self.arc_freq(i, j))

            new_fi = self.node_freq(i) + self.arc_freq(i, j)

            self.set_node_info(i, (new_mu, new_fi))

            print('Se actualizan las etiquetas de {} a ({:.2f}, {})'.format(
                i, float(new_mu), to_frac(new_fi)))

        print('\n')

        for node, info in self.node_info.items():
            m, f = float(info[0]), to_frac(info[1])
            print('{:4s}: ({:.2f}, {})'.format(node, m, f))


def to_frac(num):
    num = float(num)
    if num >= 1 or num == 0:
        return num
    div = 1

    while True:
        for i in range(div):
            if i / div == num:
                return '{}/{}'.format(i, div)
        div += 1


def estrategias_minimas(lineas, fin):

    nodes = set()
    edges = []
    appearances = defaultdict(lambda: 0)

    for l in lineas:
        wait = l[-1]
        arcs = []
        i, k, t = l[0]
        nodes.add(i)

        for j, k, t_next in l[1:-1]:
            appearances[j] += 1
            n_j = j + "'" * appearances[j]
            arcs.append((i, n_j, t, 0))
            arcs.append((j, n_j, 0, wait))
            arcs.append((n_j, j, 0, 0))
            nodes.add(n_j)
            nodes.add(j)
            i = n_j
            t = t_next

        arcs.append((i, k, t, 0))
        nodes.add(k)
        arcs[0] = arcs[0][:-1] + (wait,)
        edges.extend(arcs)

    g = Graph(nodes)
    for i, j, t, w in edges:
        g.add_edge(i, j, t, w)

    g.spiess(fin)


if __name__ == '__main__':
    # Las lineas deben ser de la forma l = [(i, j, t_ij), (j, k, t_jk), ...,
    # frecuencia]

    l1 = [('1', '2', 2), ('2', '4', 9), 3]
    l2 = [('1', '3', 7), ('3', '4', 2), 9]
    l3 = [('1', '3', 4), ('3', '4', 8), 6]
    l4 = [('3', '2', 6), ('2', '4', 5), 3]

    estrategias_minimas([l1, l2, l3, l4], '4')

