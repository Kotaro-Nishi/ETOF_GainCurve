// Fit gain = A * HV^a for each PMT and draw T/B curves, 4 segments per page.
// Usage: root -l -b -q 'gain_curve.C("pmt_gain.csv","gain_curve.pdf")'

#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <cstdio>
#include "TCanvas.h"
#include "TGraph.h"
#include "TF1.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TLine.h"
#include "TStyle.h"

const double kHV[3] = {1600., 1800., 1900.};
const double kHVref = 1800.; // fit as p0*(HV/HVref)^p1 for numerical stability

struct Fit { bool ok = false; double A = 0, a = 0, eA = 0, ea = 0; };

Fit FitGraph(TGraph *g, int color, const char *name)
{
  Fit r;
  if (!g || g->GetN() < 2) return r;
  // Initial values from a log-log linear fit
  double lx0 = std::log(g->GetX()[0] / kHVref), ly0 = std::log(g->GetY()[0]);
  double lx1 = std::log(g->GetX()[g->GetN()-1] / kHVref), ly1 = std::log(g->GetY()[g->GetN()-1]);
  double a0 = (ly1 - ly0) / (lx1 - lx0);
  double p00 = std::exp(ly0 - a0 * lx0);

  TF1 *f = new TF1(name, Form("[0]*pow(x/%f,[1])", kHVref), 1500, 2000);
  f->SetParameters(p00, a0);
  f->SetLineColor(color);
  f->SetLineWidth(2);
  g->Fit(f, "Q0");

  double p0 = f->GetParameter(0), p1 = f->GetParameter(1);
  double ep0 = f->GetParError(0), ep1 = f->GetParError(1);
  r.ok = true;
  r.a = p1;
  r.ea = ep1;
  r.A = p0 / std::pow(kHVref, p1);
  // Error propagation for A = p0 * HVref^-p1 (ignoring correlation)
  r.eA = r.A * std::sqrt(std::pow(ep0 / p0, 2) + std::pow(std::log(kHVref) * ep1, 2));
  return r;
}

void gain_curve(const char *in = "pmt_gain.csv", const char *out = "gain_curve.pdf")
{
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(1);

  // id -> {T graph, B graph}
  std::map<int, std::vector<TGraph*>> graphs;
  std::ifstream ifs(in);
  std::string line;
  std::getline(ifs, line); // header
  while (std::getline(ifs, line)) {
    std::stringstream ss(line);
    std::vector<std::string> c;
    std::string tok;
    while (std::getline(ss, tok, ',')) c.push_back(tok);
    int id = std::stoi(c[1]);
    int side = (c[2] == "T") ? 0 : 1;
    if (graphs[id].empty()) graphs[id] = {nullptr, nullptr};
    TGraph *g = new TGraph();
    for (int i = 0; i < 3; ++i) {
      double y = std::stod(c[5 + i]);
      if (!std::isnan(y)) g->SetPoint(g->GetN(), kHV[i], y);
    }
    graphs[id][side] = g;
  }

  FILE *fout = fopen("gain_fit.csv", "w");
  fprintf(fout, "seg,A,A_err,a,a_err\n");

  TCanvas *c = new TCanvas("c", "c", 1200, 900);
  c->Print(Form("%s[", out));

  int ipad = 0;
  for (auto &kv : graphs) {
    if (ipad == 0) { c->Clear(); c->Divide(2, 2); }
    c->cd(++ipad);
    gPad->SetGrid();
    gPad->SetLeftMargin(0.13);

    int id = kv.first;
    TGraph *gT = kv.second[0], *gB = kv.second[1];

    gPad->SetLogx();
    gPad->SetLogy();

    double ymin = 1e9, ymax = 0;
    for (TGraph *g : {gT, gB})
      for (int i = 0; g && i < g->GetN(); ++i) {
        ymin = std::min(ymin, g->GetY()[i]);
        ymax = std::max(ymax, g->GetY()[i]);
      }
    if (ymax <= 0) ymin = 100;

    TH1F *frame = gPad->DrawFrame(1550, ymin * 0.5, 1950, 600);
    frame->SetTitle(Form("Segment %d;HV [V];Gain [mV]", id));
    frame->GetXaxis()->SetLabelSize(0); // log x spans < 1 decade: draw labels by hand
    frame->GetXaxis()->SetTitleOffset(1.3);
    frame->GetYaxis()->SetMoreLogLabels();
    frame->GetYaxis()->SetNoExponent();
    gPad->Update();
    for (double v = 1600; v <= 1900; v += 100) {
      TLine *l = new TLine(v, ymin * 0.5, v, 600);
      l->SetLineStyle(3);
      l->Draw();
      double xndc = gPad->GetLeftMargin() + (1 - gPad->GetLeftMargin() - gPad->GetRightMargin())
                  * (std::log10(v) - gPad->GetUxmin()) / (gPad->GetUxmax() - gPad->GetUxmin());
      TLatex *t = new TLatex(xndc, gPad->GetBottomMargin() - 0.04, Form("%.0f", v));
      t->SetNDC();
      t->SetTextAlign(21);
      t->SetTextSize(0.035);
      t->SetTextFont(42);
      t->Draw();
    }

    TLegend *leg = new TLegend(0.38, 0.15, 0.88, 0.32);
    leg->SetBorderSize(1);
    leg->SetFillStyle(1001);
    leg->SetFillColor(kWhite);
    leg->SetTextSize(0.035);

    const char *sides[2] = {"T", "B"};
    const int lineColor[2] = {kRed, kBlue};
    const int marker[2] = {20, 24}; // filled / open circle
    for (int s = 0; s < 2; ++s) {
      TGraph *g = kv.second[s];
      std::string seg = Form("%d%s", id, sides[s]);
      Fit r = FitGraph(g, lineColor[s], Form("f_%s", seg.c_str()));
      if (r.ok) fprintf(fout, "%s,%.4e,%.4e,%.4f,%.4f\n", seg.c_str(), r.A, r.eA, r.a, r.ea);
      else      fprintf(fout, "%s,nan,nan,nan,nan\n", seg.c_str());
      if (!g || g->GetN() == 0) {
        leg->AddEntry((TObject*)nullptr, Form("%s: no data", seg.c_str()), "");
        continue;
      }
      g->SetMarkerStyle(marker[s]);
      g->SetMarkerColor(kBlack);
      g->SetMarkerSize(1.3);
      g->SetLineColor(lineColor[s]);
      if (r.ok) g->GetFunction(Form("f_%s", seg.c_str()))->Draw("same");
      g->Draw("P same");
      if (r.ok) leg->AddEntry(g, Form("%s: Gain = %.2e HV^{%.2f}", seg.c_str(), r.A, r.a), "pl");
      else      leg->AddEntry(g, Form("%s: fit failed", seg.c_str()), "p");
    }
    leg->Draw();

    if (ipad == 4) { c->Print(out); ipad = 0; }
  }
  if (ipad != 0) c->Print(out);
  c->Print(Form("%s]", out));
  fclose(fout);
}
