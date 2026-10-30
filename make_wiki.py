#!/usr/bin/env python3
# Generate the wiki status table from the CSV files.
# Usage: python3 make_wiki.py > wiki_status.txt

import csv
import math
import re

GAIN_COLS = ["gain_1500", "gain_1600V", "gain_1700", "gain_1800V", "gain_1900V"]

HEADER = """=== Status ===
[https://github.com/Kotaro-Nishi/ETOF_GainCurve Github]</br>
- analysis file (gain_curve.C)</br>
- visual output (gain_curve.pdf)</br>
- csv output (gain_fit.csv)</br>
{| class="wikitable"
|+ Status（click to expand）
! #Seg !! colspan="7" | Gain Curve [https://hallcweb.jlab.org/elogs/Hypernuclear+Experiments/190 ELOG#190] !! PMTs !! Cables !! Connection !! Light shield
|-
! #Seg !! Gain = <u>A</u> × HV<sup>a</sup> !! Gain = A × HV<sup><u>a</u></sup> !! 1500 V !! 1600 V !! 1700 V !! 1800 V !! 1900 V !! PMTs !! Cables !! Connection !! Light shield"""

FOOTER = """|}
spares: 10 H6612-PMT are stored in UTokyo

==== Others ====
<br><br><br>
Back to [[Hypernuclear Experiment in Hall C]]"""


def read_csv(path):
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def fmt(value, spec):
    return "--" if math.isnan(float(value)) else format(float(value), spec)


gains = read_csv("pmt_gain.csv")
fits = {r["seg"]: r for r in read_csv("gain_fit.csv")}
status = {r["seg"]: r for r in read_csv("pmt_status.csv")}

print(HEADER)
for r in gains:
    seg = r["seg"]
    cells = [fmt(fits[seg]["A"], ".2e"), fmt(fits[seg]["a"], ".2f")]
    cells += [fmt(r[c], ".0f") for c in GAIN_COLS]
    s = status[seg]
    cells += [s["pmt"], s["cables"], s["connection"], s["light_shield"]]
    line = "| " + " || ".join([seg] + cells)
    print("|-")
    print(re.sub(r"\|\|\s+(?=\|\|)", "|| ", line).rstrip())
print(FOOTER)
