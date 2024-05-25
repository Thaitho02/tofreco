#include <TFile.h>
#include <TTree.h>
#include <fstream>
#include <iostream>

void readOutputTree() {
    TFile* file = TFile::Open("/home/thaitho/tof/output/run16480_outputTree.root", "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file" << std::endl;
        return;
    }

    TTree* tree = (TTree*)file->Get("TofHitsTree");
    if (!tree) {
        std::cerr << "Error getting tree" << std::endl;
        file->Close();
        return;
    }

    int plane, bar;
    double x, y, z, signalTime, signalPosition;

    tree->SetBranchAddress("plane", &plane);
    tree->SetBranchAddress("bar", &bar);
    tree->SetBranchAddress("x", &x);
    tree->SetBranchAddress("y", &y);
    tree->SetBranchAddress("z", &z);
    tree->SetBranchAddress("signalTime", &signalTime);
    tree->SetBranchAddress("signalPosition", &signalPosition);

    std::ofstream outFile("output.txt");
    if (!outFile) {
        std::cerr << "Error creating output file" << std::endl;
        file->Close();
        return;
    }

    Long64_t entries = tree->GetEntries();
    for (Long64_t i = 0; i < entries; i++) {
        tree->GetEntry(i);
        outFile << "Entry " << i << ": "
                << "plane = " << plane << ", "
                << "bar = " << bar << ", "
                << "x = " << x << ", "
                << "y = " << y << ", "
                << "z = " << z << ", "
                << "signalTime = " << signalTime << ", "
                << "signalPosition = " << signalPosition << std::endl;
    }

    outFile.close();
    file->Close();
}
