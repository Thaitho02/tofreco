#include "TCanvas.h"
#include "TChain.h"
#include "TH1D.h"
#include "TH2F.h"
#include <cmath>
#include <vector>

void planeposition() {
  TChain *tIn = new TChain("TofHitsTree");
  tIn->Add("/home/qpham/Downloads/run16573_outputTree.root");

  std::vector<Double_t> *SignalPosition = nullptr;
  std::vector<Double_t> *SignalPlane = nullptr;

  tIn->SetBranchAddress("SignalPosition", &SignalPosition);
  tIn->SetBranchAddress("SignalPlane", &SignalPlane);

  // Khởi tạo histogram
  TH1F *h_Top = new TH1F("h_Top", "Top Signal Position", 50, -150, 150);
  TH1F *h_Bot = new TH1F("h_Bot", "Bottom Signal Position", 50, -150, 150);
  TH1F *h_Up = new TH1F("h_Up", "Up Signal Position", 50, -150, 150);
  TH1F *h_Down = new TH1F("h_Down", "Down Signal Position", 50, -150, 150);

  //  tao Histograms 2 chieu
  TH2F *hist_top_bot = new TH2F("hist_top_bot", "Top vs Bottom Plane", 50, -150,
                                150, 50, -150, 150);

  TH2F *hist_top_upstream = new TH2F("hist_top_upstream", "Top vs Upstream", 50,
                                     -150, 150, 50, -150, 150);
  TH2F *hist_top_downstream = new TH2F(
      "hist_top_downstream", "Top vs Downstream", 50, -150, 150, 50, -150, 150);

  Long64_t nEntries = tIn->GetEntries();

  // Khởi tạo mảng
  // std::vector< Double_t> *planeT = 0.0, *planeB = 0.0, *planeU = 0.0, *planeD
  // = 0.0; // Khởi tạo biến
  std::vector<Double_t> planeT(nEntries, 0.0);
  std::vector<Double_t> planeB(nEntries, 0.0);
  std::vector<Double_t> planeU(nEntries, 0.0);
  std::vector<Double_t> planeD(nEntries, 0.0);
  int count = 0;

  for (Long64_t i = 0; i < nEntries; i++) {
    tIn->GetEntry(i);

    // Tìm tọa độ x của SignalPlane top, bot, up, và down
    for (size_t j = 0; j < SignalPosition->size(); j++) {
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

  // Fill histogram với tọa độ x và y
  for (size_t i = 0; i < planeT.size(); ++i) {
    hist_top_bot->Fill(planeB[i], planeT[i]);
    hist_top_upstream->Fill(planeU[i], planeT[i]);
    hist_top_downstream->Fill(planeD[i], planeT[i]);
  }

  // Tạo một canvas và vẽ histogram 1 chiều
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
  c_planes->SaveAs("Signalplanescheck1.png");

  // Tạo một canvas và vẽ histogram 2 chiều
  TCanvas *c_hist = new TCanvas("hist", "Canvas", 1200, 800);
  c_hist->Divide(2, 2);
  c_hist->cd(1);
  hist_top_bot->Draw("COLZ");
  c_hist->cd(2);
  hist_top_downstream->Draw("COLZ");
  c_hist->cd(3);
  hist_top_upstream->Draw("COLZ");

  // lưu file
  c_hist->SaveAs("myhistcheck.png");

  // Giải phóng bộ nhớ
  delete tIn;
  delete h_Top;
  delete h_Bot;
  delete h_Up;
  delete h_Down;
}
