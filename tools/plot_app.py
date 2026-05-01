import argparse
import matplotlib.pyplot as plt
import numpy as np
import scipy


def remove_outliers(a):
    a = np.array(a)
    lower_bound, upper_bound = np.percentile(a, [25, 75])
    return a[(a >= lower_bound) & (a <= upper_bound)].tolist()


def read_benchmark(fname, average):
    times = {}
    with open(fname) as f:
        for line in f:
            n1, n2, t = (c(w) for c, w in zip((int, int, float), line.split()))
            x = (n1 + n2) / 2
            y = t / (n2 - n1)
            if x > 0:
                times.setdefault(x, []).append(y)

    if average:
        return sorted(
            (x, np.average(remove_outliers(ys))) for x, ys in times.items()
        )
    return sorted((x, y) for x, ys in times.items() for y in ys)


def extrapolate(curve, x, y):
    c, pcov = scipy.optimize.curve_fit(curve, x, y)
    xx = np.geomspace(1e6, 1e20, num=1000)
    yy = curve(xx, *c)
    return c, pcov, xx, yy, np.trapezoid(xx, yy)


def main(curve):
    parser = argparse.ArgumentParser(description='plot benchmark')
    parser.add_argument('-a', action='store_true', help='average same x')
    parser.add_argument('-f', type=str, help='benchmark file')

    args = parser.parse_args()

    xy = read_benchmark(args.f, args.a)
    
    c, pcov, xx, yy, s = extrapolate(curve, *zip(*xy))
    print('curve optimal parameters          ', c)
    print('curve optimal parameter covariance', pcov)
    print('work time estmation               ', f'{s:.3g}', 'seconds')
    print('                                  ', f'{s / 60 / 60 / 24 / 365:.3g}', 'years')

    plt.loglog(*zip(*xy), 'o')
    plt.loglog(xx, yy)
    plt.xticks([10. ** i for i in range(6, 21)])
    plt.show()
