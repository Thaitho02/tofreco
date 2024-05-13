
void readlane()
{   
    
    TChain *tIn = new TChain("TofHitsTree");
        {
            tIn->Add(TString::Format("/home/thaitho/tof/output/run16573_outputTree.root"));
        }
    tIn->SetBranchAddress("EventTimeOfFlight", &EventTimeOfFlight);
    tIn->SetBranchAddress("EventNSignals", &EventNSignals);
    tIn->SetBranchAddress("EventNHits", &EventNHits);
    tIn->SetBranchAddress("SignalType", &SignalType);
    tIn->SetBranchAddress("SignalPlane", &SignalPlane);
    tIn->SetBranchAddress("SignalBar", &SignalBar);
    tIn->SetBranchAddress("SignalPosition", &SignalPosition);
    tIn->SetBranchAddress("SignalTime", &SignalTime);
    tIn->SetBranchAddress("SignalHistListIndex", &SignalHistListIndex);
    tIn->SetBranchAddress("HitFeb", &HitFeb);
    tIn->SetBranchAddress("HitSampicChannel", &HitSampicChannel);
    tIn->SetBranchAddress("HitCell0Time", &HitCell0Time);
    tIn->SetBranchAddress("HitTotValue", &HitTotValue);
    tIn->SetBranchAddress("HitWaveform", &HitWaveform);
    tIn->SetBranchAddress("HitSampic", &HitSampic);
    tIn->SetBranchAddress("HitDaqChannel", &HitDaqChannel);
    tIn->SetBranchAddress("HitPlane", &HitPlane);
    tIn->SetBranchAddress("HitBar", &HitBar);
    tIn->SetBranchAddress("HitEdge", &HitEdge);
    tIn->SetBranchAddress("HitBaseline", &HitBaseline);
    tIn->SetBranchAddress("HitRawPeak", &HitRawPeak);
    tIn->SetBranchAddress("HitPeak", &HitPeak);
    tIn->SetBranchAddress("HitPeakSample", &HitPeakSample);
    tIn->SetBranchAddress("HitPeakTime", &HitPeakTime);
    tIn->SetBranchAddress("HitCfTime_0p1", &HitCfTime_0p1);
    tIn->SetBranchAddress("HitCfTime_0p9", &HitCfTime_0p9);
    tIn->SetBranchAddress("HitVoltageIntergral", &HitVoltageIntergral);
    tIn->SetBranchAddress("HitIsSaturated", &HitIsSaturated);
    tIn->SetBranchAddress("HitChannelOnPlane", &HitChannelOnPlane);
    tIn->SetBranchAddress("HitRawTotValue", &HitRawTotValue);
    tIn->SetBranchAddress("HitUnixTime", &HitUnixTime);
    tIn->SetBranchAddress("HitSampleLength", &HitSampleLength);
    tIn->SetBranchAddress("HitFitSuccess", &HitFitSuccess);
    tIn->SetBranchAddress("HitCfSuccess", &HitCfSuccess);
    tIn->SetBranchAddress("HitFitParameter", &HitFitParamete);
    tIn->SetBranchAddress("HitSignalIndex", &HitSignalIndex);
  
//  tao Histograms 2 chieu
    TH2F *hist_top_bot = new TH2F("hist_top_bot", "Top vs Bottom Plane", 100, -50, 50, 100, -50, 50);
    TH2F *hist_top_upstream = new TH2F("hist_top_upstream", "Top vs Upstream", 100, -50, 50, 100, -50, 50);
    TH2F *hist_top_downstream = new TH2F("hist_top_downstream", "Top vs Downstream", 100, -50, 50, 100, -50, 50);

    
    Long64_t nEntries = tIn->GetEntries();
    for (Long64_t iEntry = 0; iEntry < nEntries; ++iEntry) {
        tIn->GetEntry(iEntry);

        // Fill histograms 
        if (HitPlane == 1) { // Top Plane
            if (SignalPosition != -9999) { // Some condition to avoid invalid values
                hist_top_bot->Fill(SignalPosition, ); 
                hist_top_upstream->Fill(SignalPosition, ...); 
                hist_top_downstream->Fill(SignalPosition, ...); 
            }
        }
    }

    // Draw histograms
    TCanvas *top_vs_planes = new TCanvas("top_vs_plane", "Correlations", 800, 600);
    top_vs_planes->Divide(2, 2);
    top_vs_planes->cd(1);
    hist_top_bot->Draw("colz");
    top_vs_planes->cd(2);
    hist_top_upstream->Draw("colz");
    top_vs_planes->cd(3);
    hist_top_downstream->Draw("colz");
    top_vs_planes->Draw();
}    

