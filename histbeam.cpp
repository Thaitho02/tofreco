#include "TCanvas.h"
#include "TChain.h"
#include "TH1F.h"
#include "TH2F.h"
#include <vector>

void histbeam() {
  // Load the input TChain
  TChain *tIn = new TChain("TofHitsTree");
  tIn->Add("/home/qpham/Downloads/run16573_outputTree.root");

  // Initialize pointers for branches
  std::vector<Double_t> *SignalPosition = nullptr;
  std::vector<Double_t> *SignalPlane = nullptr;

  tIn->SetBranchAddress("SignalPosition", &SignalPosition);
  tIn->SetBranchAddress("SignalPlane", &SignalPlane);

  // Initialize histograms for signal positions
  TH1F *h_Top = new TH1F("h_Top", "Top Signal Position", 50, -150, 150);
  TH1F *h_Bot = new TH1F("h_Bot", "Bottom Signal Position", 50, -150, 150);
  TH1F *h_Up = new TH1F("h_Up", "Upstream Signal Position", 50, -150, 150);
  TH1F *h_Down =
      new TH1F("h_Down", "Downstream Signal Position", 50, -150, 150);

  // Initialize 2D histograms for correlations
  TH2F *hist_upstream_top = new TH2F("hist_upstream_top", "Up vs Top Plane", 50,
                                     -150, 150, 50, -150, 150);
  TH2F *hist_upstream_bot = new TH2F("hist_upstream_bot", "Up vs Bottom Plane",
                                     50, -150, 150, 50, -150, 150);
  TH2F *hist_upstream_downstream =
      new TH2F("hist_upstream_downstream", "Up vs Downstream Plane", 50, -150,
               150, 50, -150, 150);

  Long64_t nEntries = tIn->GetEntries();

  // Vectors to hold signal positions
  std::vector<Double_t> planeT;
  std::vector<Double_t> planeB;
  std::vector<Double_t> planeU;
  std::vector<Double_t> planeD;

  // Loop over all entries
  for (Long64_t i = 0; i < nEntries; ++i) {
    tIn->GetEntry(i);

    // Fill histograms and vectors based on SignalPlane values
    for (size_t j = 0; j < SignalPosition->size(); ++j) {
      if ((*SignalPlane)[j] == 3) {
        planeT.push_back((*SignalPosition)[j]);
        h_Top->Fill((*SignalPosition)[j]);
      } else if ((*SignalPlane)[j] == 2) {
        planeB.push_back((*SignalPosition)[j]);
        h_Bot->Fill((*SignalPosition)[j]);
      } else if ((*SignalPlane)[j] == 4) {
        planeU.push_back((*SignalPosition)[j]);
        h_Up->Fill((*SignalPosition)[j]);
      } else if ((*SignalPlane)[j] == 5) {
        planeD.push_back((*SignalPosition)[j]);
        h_Down->Fill((*SignalPosition)[j]);
      }
    }
  }

  // Fill 2D histograms
  for (size_t i = 0; i < planeT.size(); ++i) {
    // if (i < planeB.size())
      hist_upstream_bot->Fill(planeB[i], planeU[i]);
    // if (i < planeT.size())
      hist_upstream_top->Fill(planeT[i], planeU[i]);
    // if (i < planeD.size())
      hist_upstream_downstream->Fill(planeD[i], planeU[i]);
  }
  // can see that: PlaneT have smallest number ele
  std::cout << planeT.size() << std::endl;
  std::cout << planeB.size() << std::endl;
  std::cout << planeD.size() << std::endl;
  std::cout << planeU.size() << std::endl;



  // Draw and save 1D histograms
  TCanvas *c_planes = new TCanvas("planes", "Canvas", 1200, 800);
  c_planes->Divide(2, 2);
  c_planes->cd(1);
  h_Top->Draw();
  c_planes->cd(2);
  h_Bot->Draw();
  c_planes->cd(3);
  h_Up->Draw();
  c_planes->cd(4);
  h_Down->Draw();
  c_planes->SaveAs("Signalplanesbeam.png");

  // Draw and save 2D histograms
  TCanvas *c_hist = new TCanvas("hist", "Canvas", 1200, 800);
  c_hist->Divide(2, 2);
  c_hist->cd(1);
  hist_upstream_bot->Draw("COLZ");
  c_hist->cd(2);
  hist_upstream_top->Draw("COLZ");
  c_hist->cd(3);
  hist_upstream_downstream->Draw("COLZ");
  c_hist->SaveAs("myhistbeam.png");

  // Clean up
  delete tIn;
  delete h_Top;
  delete h_Bot;
  delete h_Up;
  delete h_Down;
  delete hist_upstream_top;
  delete hist_upstream_bot;
  delete hist_upstream_downstream;
}
