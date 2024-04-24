//draw plan1_plan2
   TCanvas *c_signalPlane1_vs_signalPlane2 = new TCanvas("c_signalPlane1_vs_signalPlane2", "signalPlane1_vs_signalPlane2", 900, 900);
  c_signalPlane1_vs_signalPlane2->Divide(2,2);
  c_signalPlane1_vs_signalPlane2->cd(1);
  h_TandB->Draw("HIST");
  c_signalPlane1_vs_signalPlane2->cd(2);
  h_TandU->Draw("HIST");
  c_signalPlane1_vs_signalPlane2->cd(3);
  h_TandD->Draw("HIST");
  c_allSignals->SaveAs(Form("%srun%i_signalPlane1_vs_signalPlane2.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_allSignals->SaveAs(Form("%srun%i_signalPlane1_vs_signalPlane2.pdf", output_directory.c_str(), thisRun.GetRunNumber()));

  // Fill signalPlane1 vs signalPlane2
for (auto eventit : thisRun.GetRunEventsList()) {
    for (auto signalit : eventit.GetEventSignalsList()) {
        int thisSignalType = signalit.GetSignalType();

        // Check if the signal type is relevant (assuming 3 represents relevant signals)
        if (thisSignalType == 3) {
            int signalPlane = signalit.GetSignalPlane(); // Assuming there's a method to get the signal plane

          
            if (signalPlane == kTopModule) {
                h_TandB->Fill(h_signalPosition, kBottomModule);
            } else if (signalPlane == kBottomModule) {
                h_TandB->Fill(kTopModule,h_signalPosition);
            }

            
            if (signalPlane == kTopModule) {
                h_TandU->Fill(h_signalPosition, kUpstreamModule);
            } else if (signalPlane == kUpstreamModule) {
                h_TandU->Fill(kTopModule, h_signalPosition);
            }

            // Fill histogram based on signal plane
            if (signalPlane == kTopModule) {
                h_TandD->Fill(h_signalPosition, kDownstreamModule);
            } else if (signalPlane == kDownstreamModule) {
                h_TandD->Fill(kTopModule, h_signalPosition);
            }
        }
    }
}
  // Creat Histogram for signalPlane1 vs signalPlane2
  TH2F *h_signalPlane1_vs_signalPlane2 = new TH2F("h_signalPlane1_vs_signalPlane2", Form("Signal Plane1 vs Signal Plane2, run%i", thisRun.GetRunNumber()), 50, -160, 160, 50, -160, 160);
  h_signalPlane1_vs_signalPlane2->GetXaxis()->SetTitle("Signal Plane 1 Position");
  h_signalPlane1_vs_signalPlane2->GetYaxis()->SetTitle("Signal Plane Top");
  h_signalPlane1_vs_signalPlane2->SetMarkerStyle(24);
  h_signalPlane1_vs_signalPlane2->SetMarkerColor(4);
  hist_list->Add(h_signalPlane1_vs_signalPlane2);

  TH2F *h_TandU = new TH2F("h_TandU", "Signal Plane Top vs Upstream Plane", Form("Signal Plane Top vs Upstream Plane, run%i", thisRun.GetRunNumber()),50, -160, 160, 50, -160, 160);
  h_TandU->GetXaxis()->SetTitle("Upstream Plane Position");
  h_TandU->GetYaxis()->SetTitle("Signal Plane Top");
  h_TandU->SetMarkerStyle(24);
  h_TandU->SetMarkerColor(4);
  hist_list->Add(h_TandU);

  TH2F *h_TandD = new TH2F"h_TandD", "Signal Plane Top vs Downstream Plane", Form("Signal Plane Top vs Downstream Plane, run%i", thisRun.GetRunNumber()),50, -160, 160, 50, -160, 160);
  h_TandD->GetXaxis()->SetTitle("Downstream Plane");
  h_TandD->GetYaxis()->SetTitle("Signal Plane Top");
  h_TandD->SetMarkerStyle(24);
  h_TandD->SetMarkerColor(4);
  hist_list->Add(h_TandD);

  TH2F *h_TandB = new TH2F"h_TandB", "Signal Plane Top vs Bottom Plane", Form("Signal Plane Top vs Bottom Plane, run%i", thisRun.GetRunNumber()),50, -160, 160, 50, -160, 160);
  h_TandB->GetXaxis()->SetTitle("Bottom Plane");
  h_TandB->GetYaxis()->SetTitle("Signal Plane Top");
  h_TandB->SetMarkerStyle(24);
  h_TandB->SetMarkerColor(4);
  hist_list->Add(h_TandB);
