#include "TH2F.h"
#include "TFile.h"
#include "TCanvas.h"
#include <vector>

void readplane()
{   
    
    TChain *tIn = new TChain("TofHitsTree");
        {
            tIn->Add(TString::Format("/home/thaitho/tof/output/run16573_outputTree.root"));
        }
    Double_t EventTimeOfFlight;
    Int_t EventNSignals, EventNHits;
    Int_t SignalBar ,SignalPlane, SignalType,HitFeb, HitSampicChannel, HitSampic, HitDaqChannel, HitPlane, HitBar, HitEdge,SignalHitsListIndex;
    Double_t SignalPosition;

    vector<double> *SignalPosition_T,*SignalPosition_B;

    Double_t SignalTime, HitCell0Time, HitTotValue, HitBaseline, HitRawPeak, HitPeak, HitPeakTime,HitCfTime_0p1, HitCfTime_0p9, HitVoltageIntegral, HitRawTotValue, HitUnixTime, HitSampleLength, HitPeakSample, HitChannelOnPlane, HitSignalIndex, *HitIsSaturated, HitFitSuccess, HitCfSuccess, HitWaveform, HitFitParameter;

    tIn->SetBranchAddress("TofHitsTree.EventTimeOfFlight", &EventTimeOfFlight);
    tIn->SetBranchAddress("TofHitsTree.EventNSignals", &EventNSignals);
    tIn->SetBranchAddress("TofHitsTree.EventNHits", &EventNHits);
    tIn->SetBranchAddress("TofHitsTree.SignalType", &SignalType);
    tIn->SetBranchAddress("TofHitsTree.SignalPlane", &SignalPlane);
    tIn->SetBranchAddress("TofHitsTree.SignalBar", &SignalBar);
    tIn->SetBranchAddress("TofHitsTree.SignalPosition", &SignalPosition);
    tIn->SetBranchAddress("TofHitsTree.SignalTime", &SignalTime);
    tIn->SetBranchAddress("TofHitsTree.SignalHitsListIndex", &SignalHitsListIndex);
    tIn->SetBranchAddress("TofHitsTree.HitFeb", &HitFeb);
    tIn->SetBranchAddress("TofHitsTree.HitSampicChannel", &HitSampicChannel);
    tIn->SetBranchAddress("TofHitsTree.HitCell0Time", &HitCell0Time);
    tIn->SetBranchAddress("TofHitsTree.HitTotValue", &HitTotValue);
    tIn->SetBranchAddress("TofHitsTree.HitWaveform", &HitWaveform);
    tIn->SetBranchAddress("TofHitsTree.HitSampic", &HitSampic);
    tIn->SetBranchAddress("TofHitsTree.HitDaqChannel", &HitDaqChannel);
    tIn->SetBranchAddress("TofHitsTree.HitPlane", &HitPlane);
    tIn->SetBranchAddress("TofHitsTree.HitBar", &HitBar);
    tIn->SetBranchAddress("TofHitsTree.HitEdge", &HitEdge);
    tIn->SetBranchAddress("TofHitsTree.HitBaseline", &HitBaseline);
    tIn->SetBranchAddress("TofHitsTree.HitRawPeak", &HitRawPeak);
    tIn->SetBranchAddress("TofHitsTree.HitPeak", &HitPeak);
    tIn->SetBranchAddress("TofHitsTree.HitPeakSample", &HitPeakSample);
    tIn->SetBranchAddress("TofHitsTree.HitPeakTime", &HitPeakTime);
    tIn->SetBranchAddress("TofHitsTree.HitCfTime_0p1", &HitCfTime_0p1);
    tIn->SetBranchAddress("TofHitsTree.HitCfTime_0p9", &HitCfTime_0p9);
    tIn->SetBranchAddress("TofHitsTree.HitVoltageIntegral", &HitVoltageIntegral);
    tIn->SetBranchAddress("TofHitsTree.HitIsSaturated", &HitIsSaturated);
    tIn->SetBranchAddress("TofHitsTree.HitChannelOnPlane", &HitChannelOnPlane);
    tIn->SetBranchAddress("TofHitsTree.HitRawTotValue", &HitRawTotValue);
    tIn->SetBranchAddress("TofHitsTree.HitUnixTime", &HitUnixTime);
    tIn->SetBranchAddress("TofHitsTree.HitSampleLength", &HitSampleLength);
    tIn->SetBranchAddress("TofHitsTree.HitFitSuccess", &HitFitSuccess);
    tIn->SetBranchAddress("TofHitsTree.HitCfSuccess", &HitCfSuccess);
    tIn->SetBranchAddress("TofHitsTree.HitFitParameter", &HitFitParameter);
    tIn->SetBranchAddress("TofHitsTree.HitSignalIndex", &HitSignalIndex);
  
    //  tao Histograms 2 chieu
    TH2D *hist_top_bot = new TH2D("hist_top_bot", "Top vs Bottom Plane", 100, -50, 50, 100, -50, 50);
    TH2D *hist_top_upstream = new TH2D("hist_top_upstream", "Top vs Upstream", 100, -50, 50, 100, -50, 50);
    TH2D *hist_top_downstream = new TH2D("hist_top_downstream", "Top vs Downstream", 100, -50, 50, 100, -50, 50);


    Long64_t nEntries = tIn->GetEntries();
    for (Long64_t iEntry =1; iEntry < nEntries; ++iEntry) {
        tIn->GetEntry(iEntry);

        //hist_top_bot->Fill(1.0,1.0);
        //hist_top_upstream->Fill(1.0, 1.0);
        //hist_top_downstream->Fill(1.0,1.0);
         // Lưu trữ tọa độ x và y
        Double_t xPos = 0.0;
        Double_t yPos = 0.0;

        //toa do

        for (size_t j = 0; j < SignalPosition->size(); j++)
        {
        if (SignalPlane == 3) { // Top Plane
            SignalPosition_T[iEntry] =SignalPosition = yPos;
                }
        //    if (SignalPosition != -9999) { // Some condition to avoid invalid values
        else if (SignalPlane == 2) { //Botom plane
            SignalPosition_B[iEntry] =SignalPosition = xPos;
                }
                hist_top_bot->Fill(SignalPosition_T[iEntry],SignalPosition_B[iEntry]);
                // hist_top_upstream->Fill(SignalPosition, HitPlane); 
                // hist_top_downstream->Fill(SignalPosition, SignalTime); 
        //    }
     }
    }

    // Draw histograms
    TCanvas *top_vs_planes = new TCanvas("top_vs_plane", "Correlations", 800, 600);

    top_vs_planes->Divide(2, 2);
    top_vs_planes->cd(1);
    hist_top_bot->Draw("colz");
    //top_vs_planes->cd(2);
    //hist_top_upstream->Draw("colz");
    //top_vs_planes->cd(3);
   // hist_top_downstream->Draw("colz");
   // top_vs_planes->Draw();

    // save to file
    TFile *tfile = new TFile("mychart.root","RECREATE");
   // hist_top_upstream->Write();
   // hist_top_downstream->Write();
    hist_top_bot->Write();
    tfile->Close();
    delete hist_top_upstream;
    delete tIn;
    delete top_vs_planes;
} 
