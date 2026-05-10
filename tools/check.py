import argparse
import os
import sys


def read_table(f):
    def ranges():
        for line in f:
            n1, n2, *_ = line.split()
            yield int(n1), int(n2)

    return sorted(set(ranges()))


def expand(pattern, n1, n2):
    return pattern \
        .replace('{}', str(n1), count=1) \
        .replace('{}', str(n2), count=1)


def main():
    parser = argparse.ArgumentParser(description='sv-baseline benchmark')
    parser.add_argument('-c1', help='first command')
    parser.add_argument('-c2', help='second command')

    args = parser.parse_args()

    for n1, n2 in read_table(sys.stdin):
        e1 = expand(args.c1, n1, n2)
        e2 = expand(args.c2, n1, n2)
        command = f'bash -c "diff <({e1}) <({e2})"'
        print(command)
        ec = os.system(command)
        if ec != 0:
            print(f'error on {n1} {n2}', file=sys.stderr)
            print(f'    {e1}', file=sys.stderr)
            print(f'    {e2}', file=sys.stderr)
            exit(1)


main()
