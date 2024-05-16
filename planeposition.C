void planeposition()
{
    TChain *tIn = new TChain("TofHitsTree");
    tIn->Add("/home/thaitho/tof/output/run16573_outputTree.root");

    std::vector<Double_t> *SignalPosition = nullptr;
    std::vector<Double_t> *SignalPlane = nullptr;

    

    tIn->SetBranchAddress("SignalPosition", &SignalPosition);
    tIn->SetBranchAddress("SignalPlane", &SignalPlane);

    // Khởi tạo histogram
    TH1F *h_Top = new TH1F("h_Top", "Top Signal Position", 100,0, 100);
    TH1F *h_Bot = new TH1F("h_Bot", "Bottom Signal Position", 100,0, 100);
    TH1F *h_Up = new TH1F("h_Up", "Up Signal Position", 100,0, 100);
    TH1F *h_Down = new TH1F("h_Down", "Down Signal Position", 100,0, 100);

    Long64_t nEntries = tIn->GetEntries();
     
 // Khởi tạo mảng
    //std::vector< Double_t> *planeT = 0.0, *planeB = 0.0, *planeU = 0.0, *planeD = 0.0; // Khởi tạo biến 
    std::vector<Double_t> planeT(nEntries, 0.0);
    std::vector<Double_t> planeB(nEntries, 0.0);
    std::vector<Double_t> planeU(nEntries, 0.0);
    std::vector<Double_t> planeD(nEntries, 0.0);

    for (Long64_t i = 0; i < nEntries; i++)
    {
        tIn->GetEntry(i);


        // Tìm tọa độ x của SignalPlane top, bot, up, và down
        for (size_t j = 0; j < SignalPosition->size(); j++)
        {
            if ((*SignalPlane)[j] == 3)
            {
                planeT[i] = (*SignalPosition)[j];
                h_Top->Fill(planeT[i]);
            }
            else if ((*SignalPlane)[j] == 2)
            {
                planeB[i] = (*SignalPosition)[j];
                h_Bot->Fill(planeB[i]);
            }
            else if ((*SignalPlane)[j] == 4)
            {
                planeU[i] = (*SignalPosition)[j];
                h_Up->Fill(planeU[i]);
            }
            else if ((*SignalPlane)[j] == 5)
            {
                planeD[i] = (*SignalPosition)[j];
                h_Down->Fill(planeD[i]);
            }
        }

        // Fill histogram với tọa độ x và y
     
    }

    // Tạo một canvas và vẽ histogram
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
    c_planes->SaveAs("Signalplanes.png");

    // Giải phóng bộ nhớ
    delete tIn;
    delete h_Top;
    delete h_Bot;
    delete h_Up;
    delete h_Down;
}

