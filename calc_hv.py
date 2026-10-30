#!/usr/bin/env python3
# Calculate the HV of each PMT that gives the requested signal height.
# HV = (height / A)^(1/a), with A and a taken from gain_fit.csv.
# Usage: python3 calc_hv.py

import csv
import math

HV_POINTS = {
    "gain_1500": 1500,
    "gain_1600V": 1600,
    "gain_1700": 1700,
    "gain_1800V": 1800,
    "gain_1900V": 1900,
}


def read_csv(path):
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


height = float(input("Enter target signal height [mV] > "))
gains = {r["seg"]: r for r in read_csv("pmt_gain.csv")}
out_path = f"hv_{height:g}mV.csv"

with open(out_path, "w", newline="", encoding="utf-8") as f:
    w = csv.writer(f)
    w.writerow(["seg", "HV", "note"])
    print(f"{'seg':>4} {'HV [V]':>7}  note")
    for r in read_csv("gain_fit.csv"):
        seg, A, a = r["seg"], float(r["A"]), float(r["a"])
        if math.isnan(A):
            w.writerow([seg, "nan", "no fit"])
            print(f"{seg:>4} {'--':>7}  no fit")
            continue
        hv = (height / A) ** (1 / a)
        # Flag values outside the HV range this channel was measured in
        points = [v for k, v in HV_POINTS.items() if float(gains[seg][k]) == float(gains[seg][k])]
        note = "" if min(points) <= hv <= max(points) else "extrapolated"
        w.writerow([seg, f"{hv:.0f}", note])
        print(f"{seg:>4} {hv:>7.0f}  {note}")

print(f"Saved to {out_path}")
