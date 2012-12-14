#!/usr/bin/env python

import sys

def main():

    nodes = set()
    start = int(sys.argv[2])
    size = int(sys.argv[3])
    count = 0

    with open(sys.argv[1]) as f:
        for line in f:
            parts = line.split()
            u = int(parts[0])
            v = int(parts[1])

            count += 1
            if count <= start: continue

            if len(nodes) < size:
                nodes.add(u)
                nodes.add(v)
                print "%s %s" % (u, v)
            elif u in nodes and v in nodes:
                print "%s %s" % (u, v)


if __name__ == "__main__":
    main()

