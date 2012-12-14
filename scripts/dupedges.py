#!/usr/bin/env python

import sys

def main():

    edges = set()

    with open(sys.argv[1]) as f:
        for line in f:
            parts = line.split()
            u = int(parts[0])
            v = int(parts[1])

            if (u, v) in edges:
                print "F %s %s" % (u, v)
            else:
                edges.add((v, u))
                print "N %s %s" % (u, v)


if __name__ == "__main__":
    main()

