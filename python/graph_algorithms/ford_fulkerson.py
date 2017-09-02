from collections import defaultdict


class Edge:

    def __init__(self, num, start, end, cost, capacity):
        self.num = num
        self.start = start
        self.end = end
        self.cost = cost
        self.capacity = capacity
        self.flow = 0

    @property
    def residual(self):
        return self.capacity - self.flow


class Graph:

    def __init__(self):
        self._out = defaultdict(lambda: [])
        self._in = defaultdict(lambda: [])
        self.edges = []

    def add_edge(self, num, start, end, cost, capacity):
        edge = Edge(num, start, end, cost, capacity)
        self._out[start].append(edge)
        self._in[end].append(edge)

    def _get_path(self, start, end, path):
        if start == end:
            return path
        for edge in sorted(self._out[start], key=lambda e: -e.residual):
            if edge.residual > 0 and edge not in path:
                result = self._get_path(edge.end, end, path + [edge])
                if result is not None:
                    return result

    def max_flow(self, start, end):
        path = self._get_path(start, end, [])
        while path is not None:
            flow = min(e.residual for e in path)
            for e in path:
                e.flow += flow
            path = self._get_path(start, end, [])
        return sum(e.flow for e in self._out[start])

    @classmethod
    def from_file(cls, filename):
        g = cls()
        with open(filename, "r") as f:
            for edge in f:
                num, start, end, cost, capacity = edge.strip().split()
                g.add_edge(num, start, end, float(cost), int(capacity))
        return g

if __name__ == "__main__":
    g = Graph.from_file("arcos.dat")
    print(g.max_flow('B', 'Q'))
