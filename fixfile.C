void c()
{
    TChain *tIn = new TChain("TofHitsTree");
    tIn->Add("/home/thaitho/tof/output/run16573_outputTree.root");

    std::vector<Double_t> *SignalTime = nullptr;
    std::vector<Double_t> *SignalPlane = nullptr;

    tIn->SetBranchAddress("SignalPosition", &SignalPosition);
    tIn->SetBranchAddress("SignalPlane", &SignalPlane);

    // Khởi tạo histogram
    TH2F *h = new TH2F("h", "Signal Position Graph", 100, 0, 10000000000, 100, 0, 10000000000);

    Long64_t nEntries = tIn->GetEntries();

    for (Long64_t i = 0; i < nEntries; i++)
    {
        tIn->GetEntry(i);

        // Lưu trữ tọa độ x và y
        Double_t xPos = 0.0;
        Double_t yPos = 0.0;
        
        // Tìm tọa độ x của SignalPlane 3 và tọa độ y của SignalPlane 4
        for (size_t j = 0; j < SignalTime->size(); j++)
        {
            if ((*SignalPlane)[j] == 2)
            {
                xPos = (*SignalTime)[j];
            }
            else if ((*SignalPlane)[j] == 3)
            {
                yPos = (*SignalTime)[j];
            }

            h->Fill(xPos, yPos);
        }

        // Fill histogram với tọa độ x và y
     
    }

    // Tạo một canvas và vẽ histogram
    TCanvas *c = new TCanvas("c", "Canvas", 800, 600);
    h->Draw("COLZ"); // Sử dụng "COLZ" để vẽ histogram 2D

    c->SaveAs("SignalTimeGraph.png");

    delete tIn; // Giải phóng bộ nhớ
    delete h;
}