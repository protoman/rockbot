"""Weighted palette fit shared by the converters (BUG-004).

fit(weights, n): `weights` is {(r,g,b): weight} with colours already on the
MD grid; returns n-or-fewer colours chosen by Ward agglomeration (merge the pair
whose union adds the least weighted squared error) refined by k-means with the
centroids snapped to the MD grid. error() prices any palette the same way, so a
converter can keep whichever of two fits is better.
"""
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def _d2(a, b):
    return sum((a[k] - b[k]) ** 2 for k in range(3))


def _pal_error(weight, uniq):
    err = 0
    for c, n in weight.items():
        err += min(_d2(c, p) for p in uniq) * n
    return err


def _ward_merge(weight, limit):
    """Greedy agglomeration: merge the pair whose union raises the weighted
    squared error least (Ward: wa*wb/(wa+wb) * d^2), so two big areas of similar
    shade fold together before a rare, distinct shade is touched."""
    weight = dict(weight)
    while len(weight) > limit:
        cols = list(weight)
        best, pair = None, None
        for i in range(len(cols)):
            for j in range(i + 1, len(cols)):
                a, b = cols[i], cols[j]
                wa, wb = weight[a], weight[b]
                cost = wa * wb / (wa + wb) * _d2(a, b)
                if best is None or cost < best:
                    best, pair = cost, (a, b)
        a, b = pair
        wa, wb = weight.pop(a), weight.pop(b)
        merged = tuple(md_round((a[k] * wa + b[k] * wb) / (wa + wb))
                       for k in range(3))
        weight[merged] = weight.get(merged, 0) + wa + wb
    return list(weight)


def _lloyd(weight, uniq, rounds=12):
    """k-means refinement of the Ward result, centroids snapped to the MD grid.
    Keeps the best palette seen (snapping can make a round slightly worse)."""
    best, best_err = list(uniq), _pal_error(weight, uniq)
    cur = list(uniq)
    for _ in range(rounds):
        acc = [[0, 0, 0, 0] for _ in cur]
        for c, n in weight.items():
            k = min(range(len(cur)), key=lambda i: _d2(c, cur[i]))
            for ch in range(3):
                acc[k][ch] += c[ch] * n
            acc[k][3] += n
        nxt = [tuple(md_round(a[ch] / a[3]) for ch in range(3)) if a[3]
               else cur[i] for i, a in enumerate(acc)]
        if nxt == cur:
            break
        cur = nxt
        err = _pal_error(weight, cur)
        if err < best_err:
            best, best_err = list(cur), err
    return best



def error(weight, palette):
    """Weighted mean squared error of `palette` (list of rgb) over `weight`."""
    tot = sum(weight.values())
    return _pal_error(weight, palette) / max(1, tot) if palette else 0


def fit(weight, n):
    """Best <= n colours for `weight`. Colours already <= n are returned as is."""
    weight = dict(weight)
    if len(weight) <= n:
        return list(weight)
    return _lloyd(weight, _ward_merge(weight, n))
