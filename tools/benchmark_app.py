import argparse
import os
import time


def elapsed(n1, n2, command):
    start = time.perf_counter()
    os.system(command(n1, n2))
    finish = time.perf_counter()
    return finish - start


def read_table(fname):
    table = {}
    if os.path.isfile(fname):
        with open(fname) as f:
            for line in f:
                n1, n2, t = line.split()
                n1 = int(n1)
                n2 = int(n2)
                t = float(t)
                table.setdefault((n1, n2), []).append(t)
    return table


def update_table(fname, n1, n2, t):
    with open(fname, 'a') as f:
        print(n1, n2, t, file=f)


def main(scheme, command):
    parser = argparse.ArgumentParser(description='sv-baseline benchmark')
    parser.add_argument(
        '-n',
        action='store_true',
        help='dry run, do not update benchmark file'
    )
    parser.add_argument('-f', type=str, help='benchmark file')

    args = parser.parse_args()

    table = read_table(args.f)

    for key, k in scheme.items():
        k_now = len(table.get(key, []))
        for _ in range(k - k_now): 
            print(*key, '...')
            if not args.n:
                update_table(args.f, *key, elapsed(*key, command))
