 TH2F *h_TvsB = new TH2F("h_TvsB", Form("TvsB, run%i", thisRun.GetRunNumber()), 1, -110, 110, 20, -0.5, 19.5);
  h_TvsB->GetXaxis()->SetTitle("top");
  h_TvsB->GetYaxis()->SetTitle("bottom");
  h_TvsB->SetMarkerStyle(24);
  h_TvsB->SetMarkerColor(4);
  hist_list->Add(h_TvsB);

  TH2F *h_TvsU = new TH2F("h_TvsU", Form("h_TvsU, run%i", thisRun.GetRunNumber()), 1, -110, 110, 20, -0.5, 19.5);
  h_TvsU->GetXaxis()->SetTitle("top");
  h_TvsU->GetYaxis()->SetTitle("up");
  h_TvsU->SetMarkerStyle(24);
  h_TvsU->SetMarkerColor(4);
  hist_list->Add(h_TvsU);

TH2F *h_TvsD = new TH2F("h_TvsD", Form("h_TvsD, run%i", thisRun.GetRunNumber()), 1, -110, 110, 20, -0.5, 19.5);
  h_TvsD->GetXaxis()->SetTitle("top");
  h_TvsD->GetYaxis()->SetTitle("down");
  h_TvsD->SetMarkerStyle(24);
  h_TvsD->SetMarkerColor(4);
  hist_list->Add(h_TvsD);
