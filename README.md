# ETOF_GainCurve
ETOF Gain Curve analysis codes

Fit `Gain = A * HV^a` for each PMT (T/B of segments 1-58) using gains measured at 1500 - 1900 V (not every channel has all five points).

## Files
- `pmt_gain.csv` : input table (`nan` = no data)
- `gain_curve.C` : ROOT macro (fit + plots)
- `gain_curve.pdf` : gain curves, 4 segments per page
- `gain_fit.csv` : fit results (A, a and errors)
- `pmt_status.csv` : PMT, cable, connection and light shield status for the wiki table
- `make_wiki.py` : generates the wiki table (A, a from `gain_fit.csv`, gains from `pmt_gain.csv`, status from `pmt_status.csv`)
- `wiki_status.txt` : generated wiki table

## Usage
```
root -l -b -q 'gain_curve.C("pmt_gain.csv","gain_curve.pdf")'
python3 make_wiki.py > wiki_status.txt
```
