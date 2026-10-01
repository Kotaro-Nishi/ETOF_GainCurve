# ETOF_GainCurve
ETOF Gain Curve analysis codes

Fit `Gain = A * HV^a` for each PMT (T/B of segments 1-58) using gains measured at 1600 / 1800 / 1900 V.

## Files
- `pmt_gain.csv` : input table (`nan` = no data)
- `gain_curve.C` : ROOT macro (fit + plots)
- `gain_curve.pdf` : gain curves, 4 segments per page
- `gain_fit.csv` : fit results (A, a and errors)

## Usage
```
root -l -b -q 'gain_curve.C("pmt_gain.csv","gain_curve.pdf")'
```
