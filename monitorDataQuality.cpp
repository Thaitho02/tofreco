//
// Created by E. Villa on Mar 28th, 2023. 
// emanuele.villa@cern.ch
// 

// #include "CommonUtils.h"
// #include "TofHit.h"
#include "TofRun.h"

#include "TApplication.h"
#include "TH1F.h"

#include "GenericToolbox.h"
#include "GenericToolbox.RawDataArray.h"
#include "CmdLineParser.h"
#include "Logger.h"

#include "string"
#include "iostream"
#include "vector"

#include "nlohmann/json.hpp"

// move
#include "TLegend.h"

LoggerInit([]{
  Logger::getUserHeader() << "[" << FILENAME << "]";
});


int main(int argc, char *argv[]){

  CmdLineParser clp;

  clp.getDescription() << "> monitorDataQuality is the main interface for unfolding TOF data (DAQ format)"<< std::endl;
  clp.getDescription() << "> and performs a primary event reconstruction, mainly for monitoring." << std::endl;

  clp.addDummyOption("Main options");
  clp.addOption("runNumber", {"-n", "--run-number"}, "Specify run number.");
  clp.addOption("whichSoftware", {"-w", "--which-software"}, "Run taken with either Windows or Linux software.");
  clp.addOption("appSettings", {"-s", "--app-settings"}, "Specify application settings file path.");
  clp.addOption("runFullPath", {"-r", "--run"}, "Run full path.");
  clp.addOption("outputDir", {"-o", "--output"}, "Specify output directory path");

  clp.addDummyOption("Triggers");
  clp.addTriggerOption("verboseMode", {"-v"}, "RunVerboseMode, bool");

  clp.addDummyOption();

  // usage always displayed
  LogInfo << clp.getDescription().str() << std::endl;

  LogInfo << "Usage: " << std::endl;
  LogInfo << clp.getConfigSummary() << std::endl << std::endl;

  clp.parseCmdLine(argc, argv);

  LogThrowIf( clp.isNoOptionTriggered(), "No option was provided." );

  LogInfo << "Provided arguments: " << std::endl;
  LogInfo << clp.getValueSummary() << std::endl << std::endl;
  
  std::string software = std::string(clp.getOptionVal<std::string>("whichSoftware", "linux")); // if no arg, deafult is linux
  if (software == "linux") LogInfo << "Linux software" << std::endl;
  else if (software == "windows") LogInfo << "Windows software" << std::endl;
  else{
    LogError << "Parsed software not recognized, has to be 'windows' or 'linux'" << std::endl;
    return 1;
  }

  // read settings from json file
  std::string ana_settings_file = clp.getOptionVal<std::string>("appSettings"); 
  std::ifstream ana_settings_stream(ana_settings_file);
  LogInfo << "Reading analysis settings from "   << ana_settings_file.c_str() << std::endl;
  nlohmann::json analysis_settings_file;
  // check integrity of json file
  if (!ana_settings_stream.is_open()) {
    LogError << "Analysis settings file not found. Make sure to use an absolute path." << std::endl;
    return 1;
  }

  ana_settings_stream >> analysis_settings_file;

  bool run_root_app = analysis_settings_file["DisplayPlots"]; // display plots in a root app
  std::cout << "Run root app: " << run_root_app << std::endl;
  bool waveform_display = analysis_settings_file["HitWaveformDisplay"];
  std::cout << "Waveform display: " << waveform_display << std::endl;
 
  TofRun thisRun;

  auto run_full_path = clp.getOptionVal<std::string>("runFullPath");
  auto output_directory = clp.getOptionVal<std::string>("outputDir");

  LogInfo << "Run path " << run_full_path << std::endl;

  thisRun.RunSetVerbose( clp.isOptionTriggered("verboseMode") ); // TODO check this
  thisRun.RunSetSoftwareType(software);
  thisRun.RunSetInputFilePath( run_full_path );
  thisRun.RunReadFilename();
  thisRun.RunSaveSettings();
  thisRun.RunQualityCheck();
  thisRun.RunSetAnalysisOptions(ana_settings_file);
  thisRun.RunGetInfo(); // comment to reduce output
  thisRun.RunLoadHits();
  thisRun.RunPrintErrors();
  // this is not the app to do this, but in case it's possible to save the tree. 
  // It is not recommended since there can be two copies of this file read by other apps
  // meaning one coming from here and one coming from createOutputTree
  // A solution could be to pass the filename from here and not only the path
  // thisRun.RunWriteFlatRootTree(output_directory);


  //////////////////////////////////////////////////////////////
  // ROOT app and objects to read the data in the Run
  auto *app = new TApplication("myapp", &argc, argv);
  auto *hist_list = new TObjArray();

  TH1F *h_signalBar = new TH1F("h_signalBar", Form("SignalBar, run%i",thisRun.GetRunNumber()), 20, -0.5, 19.5);
  h_signalBar->GetXaxis()->SetTitle("Bar");
  h_signalBar->SetMinimum(0);
  hist_list->Add(h_signalBar);

  // not elegant, temporary solution
  // std::vector <std::string> PlaneLabels = {"U","D","T","B","L","R"}; // use map instead! -> done, moved to map
  // std::map<std::string, int> PlaneNumbers;
  // for (int i= 0; i < 6; i++) PlaneNumbers[PlaneLabels[i]] = i;

  // create a vector of 6 histograms called h_signalBar_plane, one for each plane
  std::vector <TH1F*> h_signalBar_plane;
  h_signalBar_plane.reserve(6);
  for (int i = 0; i < 6; i++){
    h_signalBar_plane[i] = new TH1F(Form("h_signalBar_plane%s", PlaneLabels.at(i).c_str()), Form("SignalBar, run%i, plane%s", thisRun.GetRunNumber(),  PlaneLabels.at(i).c_str()), 20, -0.5, 19.5);
    h_signalBar_plane[i]->GetXaxis()->SetTitle("Bar");
    h_signalBar_plane[i]->SetMinimum(0);
    hist_list->Add(h_signalBar_plane[i]);
  }

  TH1F *h_signalPlane = new TH1F("h_signalPlane", Form("SignalPlane, run%i", thisRun.GetRunNumber()), 6, -0.5, 5.5);
  h_signalPlane->GetXaxis()->SetTitle("Plane");
  h_signalPlane->SetMinimum(0);
  hist_list->Add(h_signalPlane);

  TH1F *h_signalPosition = new TH1F("h_signalPosition", Form("SignalPosition, run%i", thisRun.GetRunNumber()), 50, -150, 150.);
  h_signalPosition->GetXaxis()->SetTitle("Position [cm]");
  h_signalPosition->SetMinimum(0);
  hist_list->Add(h_signalPosition);

  // create a vector of 6 histograms called h_signalPosition_plane, one for each plane
  std::vector <TH1F*> h_signalPosition_plane;
  h_signalPosition_plane.reserve(6);
  for (int i = 0; i < 6; i++){
    h_signalPosition_plane[i] = new TH1F(Form("h_signalPosition_plane%s", PlaneLabels.at(i).c_str()), Form("SignalPosition, run%i, plane%s", thisRun.GetRunNumber(), PlaneLabels.at(i).c_str()), 50, -150, 150.);
    h_signalPosition_plane[i]->GetXaxis()->SetTitle("Position [cm]");
    h_signalPosition_plane[i]->SetMinimum(0);
    hist_list->Add(h_signalPosition_plane[i]);
  }


  TH1F *h_hitPeak = new TH1F("h_hitPeak", Form("HitPeak, run%i", thisRun.GetRunNumber()), 200, -0.1,1.1 );
  h_hitPeak->GetXaxis()->SetTitle("Peak [V]");
  h_hitPeak->SetMinimum(0);
  hist_list->Add(h_hitPeak);

  // create a vector of 6 histograms called h_hitPeak_plane, one for each plane
  std::vector <TH1F*> h_hitPeak_plane;
  h_hitPeak_plane.reserve(6);
  for (int i = 0; i < 6; i++){
    h_hitPeak_plane[i] = new TH1F(Form("h_hitPeak_plane%s", PlaneLabels.at(i).c_str()), Form("HitPeak, run%i, plane%s", thisRun.GetRunNumber(), PlaneLabels.at(i).c_str()), 150, -0.1,1.1 );
    h_hitPeak_plane[i]->GetXaxis()->SetTitle("Peak [V]");
    h_hitPeak_plane[i]->SetMinimum(0);
    hist_list->Add(h_hitPeak_plane[i]);
  }
  

  TH1F *h_singleHitPeak = new TH1F("h_singleHitPeak", Form("SingleHitPeak, run%i", thisRun.GetRunNumber()), 50, -0.1,1.1 );
  h_singleHitPeak->GetXaxis()->SetTitle("Peak [V]");
  h_singleHitPeak->SetMinimum(0);
  hist_list->Add(h_singleHitPeak);

  // TH1F to plot channels firing in a run. On x axis channel number, on y axis number of times that channel fired
  TH1F *h_channelsFiring = new TH1F("h_channelsFiring", Form("ChannelsFiring, run%i", thisRun.GetRunNumber()), 256, -0.5, 255.5);
  h_channelsFiring->GetXaxis()->SetTitle("Channel");
  h_channelsFiring->SetMinimum(0);
  hist_list->Add(h_channelsFiring);

  // create a vector of 6 histograms called h_channelsFiring_plane, one for each plane
  std::vector <TH1F*> h_channelsFiring_plane;
  h_channelsFiring_plane.reserve(6);
  for (int i = 0; i < 6; i++){
    h_channelsFiring_plane[i] = new TH1F(Form("h_channelsFiring_plane%s", PlaneLabels.at(i).c_str()), Form("ChannelsFiring, run%i, plane%s", thisRun.GetRunNumber(), PlaneLabels.at(i).c_str()), 40, -0.5, 39.5);
    h_channelsFiring_plane[i]->GetXaxis()->SetTitle("Channel");
    h_channelsFiring_plane[i]->SetMinimum(0);
    hist_list->Add(h_channelsFiring_plane[i]);
  }

  // array of histos to plot the baseline, rising time, peak sample, max amplitude, for each channel
  const int nChannels = TofSampicParameters::nChannels; 
  TH1F *h_baseline[nChannels];
  TH1F *h_maxAmp[nChannels];
  TH1F *h_peakSample [nChannels];
  TH1F *h_risingTime [nChannels];
  TH1F *h_integral [nChannels];
  for (int i = 0; i < nChannels; i++){
    h_baseline[i] = new TH1F(Form("h_baseline_%i", i), Form("Baseline, run%i, channel%i", thisRun.GetRunNumber(), i), 50, -0.2, 0.2);
    h_baseline[i]->GetXaxis()->SetTitle("Baseline [V]");
    hist_list->Add(h_baseline[i]);

    h_maxAmp[i] = new TH1F(Form("h_maxAmp_%i", i), Form("MaxAmp, run%i, channel%i", thisRun.GetRunNumber(), i), 50, -0.1, 1.1);
    h_maxAmp[i]->GetXaxis()->SetTitle("Max Amplitude [V]");
    hist_list->Add(h_maxAmp[i]);

    h_peakSample[i] = new TH1F(Form("h_peakSample_%i", i), Form("PeakSample, run%i, channel%i", thisRun.GetRunNumber(), i), 50, 0, 64);
    h_peakSample[i]->GetXaxis()->SetTitle("Peak Sample [V]");
    hist_list->Add(h_peakSample[i]);

    h_risingTime[i] = new TH1F(Form("h_risingTime_%i", i), Form("RisingTime, run%i, channel%i", thisRun.GetRunNumber(), i), 50, 0, 20);
    h_risingTime[i]->GetXaxis()->SetTitle("Rising Time [V]");
    hist_list->Add(h_risingTime[i]);

    h_integral[i] = new TH1F(Form("h_integral_%i", i), Form("Integral, run%i, channel%i", thisRun.GetRunNumber(), i), 50, 0, 500);
    h_integral[i]->GetXaxis()->SetTitle("Integral [V]");
    hist_list->Add(h_integral[i]);

  }


  // TH1F to plot time of flight
  TH1F *h_timeOfFlight = new TH1F("h_timeOfFlight", Form("TimeOfFlight, run%i", thisRun.GetRunNumber()), 40, -0.5, 55.5);
  h_timeOfFlight->GetXaxis()->SetTitle("Time of Flight [ns]");
  h_timeOfFlight->SetMinimum(0);
  hist_list->Add(h_timeOfFlight);

  TH1F *h_saturatedHits = new TH1F("h_saturatedHits", Form("SaturatedHits, run%i", thisRun.GetRunNumber()), 256, -0.5, 255.5);
  h_saturatedHits->GetXaxis()->SetTitle("Channel");
  h_saturatedHits->SetMinimum(0);
  h_saturatedHits->SetFillColor(kRed);
  h_saturatedHits->SetLineColor(kRed);
  h_saturatedHits->SetFillStyle(3004);
  hist_list->Add(h_saturatedHits);

  // create a vector of 6 histograms called h_saturatedHits_plane, one for each plane
  std::vector <TH1F*> h_saturatedHits_plane;
  h_saturatedHits_plane.reserve(6);
  for (int i = 0; i < 6; i++){
    h_saturatedHits_plane[i] = new TH1F(Form("h_saturatedHits_plane%s", PlaneLabels.at(i).c_str()), Form("SaturatedHits, run%i, plane%s", thisRun.GetRunNumber(), PlaneLabels.at(i).c_str()), 256, -0.5, 255.5);
    h_saturatedHits_plane[i]->GetXaxis()->SetTitle("Channel");
    h_saturatedHits_plane[i]->SetMinimum(0);
    h_saturatedHits_plane[i]->SetFillColor(kRed);
    h_saturatedHits_plane[i]->SetLineColor(kRed);
    h_saturatedHits_plane[i]->SetFillStyle(3004);
    hist_list->Add(h_saturatedHits_plane[i]);
  }


  TH1F * h_saturatedOtherEdge = new TH1F("h_saturatedOtherEdge", Form("SaturatedOtherEdge, run%i", thisRun.GetRunNumber()), 256, -0.5, 255.5);
  h_saturatedOtherEdge->GetXaxis()->SetTitle("Channel");
  h_saturatedOtherEdge->SetMinimum(0);
  h_saturatedOtherEdge->SetLineColor(kGreen);
  h_saturatedOtherEdge->SetFillStyle(3004);
  hist_list->Add(h_saturatedOtherEdge);

  // create a vector of 6 histograms called h_saturatedOtherEdge_plane, one for each plane
  std::vector <TH1F*> h_saturatedOtherEdge_plane;
  h_saturatedOtherEdge_plane.reserve(6);
  for (int i = 0; i < 6; i++){
    h_saturatedOtherEdge_plane[i] = new TH1F(Form("h_saturatedOtherEdge_plane%s", PlaneLabels.at(i).c_str()), Form("SaturatedOtherEdge, run%i, plane%s", thisRun.GetRunNumber(), PlaneLabels.at(i).c_str()), 256, -0.5, 255.5);
    h_saturatedOtherEdge_plane[i]->GetXaxis()->SetTitle("Channel");
    h_saturatedOtherEdge_plane[i]->SetMinimum(0);
    h_saturatedOtherEdge_plane[i]->SetLineColor(kGreen);
    h_saturatedOtherEdge_plane[i]->SetFillStyle(3004);
    hist_list->Add(h_saturatedOtherEdge_plane[i]);
  }

  // display the events


  std::vector <TH2F*> h_planes; // better static?
  h_planes.reserve(6);
  for (int i = 0; i < 6; i++){
    h_planes[i] = new TH2F(Form("plane%s", PlaneLabels.at(i).c_str()), Form("plane%s", PlaneLabels.at(i).c_str()), 1, -110, 110, 20, -0.5, 19.5);
    h_planes[i]->SetStats(false);
    // h_planes[i]->GetXaxis()->SetTickLength(0);
    // h_planes[i]->GetXaxis()->SetLabelOffset(999);
    h_planes[i]->GetYaxis()->SetTickLength(0);
    h_planes[i]->GetYaxis()->SetTitle("Bar number");
    h_planes[i]->GetXaxis()->SetTitle("Position [cm]");
    hist_list->Add(h_planes[i]);
  }
  std::vector <TGraphErrors*> g_hits;
  g_hits.reserve(6);
  for (int i = 0; i < 6; i++){
    g_hits[i] = new TGraphErrors();
    g_hits[i]->SetTitle(Form("hits_plane%s", PlaneLabels.at(i).c_str()));
    g_hits[i]->SetMarkerStyle(22);
    g_hits[i]->SetMarkerSize(0.6);
    g_hits[i]->SetMarkerColor(2);
    // g_hits[i]->SetStats(false);
    // g_hits[i]->GetXaxis()->SetTickLength(0);
    // g_hits[i]->GetXaxis()->SetLabelOffset(999);
    // g_hits[i]->GetYaxis()->SetTickLength(0);
  }

  // create a vector of histograms with 256 elements, each being a vector of 62 elements
  int nSamples = thisRun.GetRunNSamplesInWaveform();
  // print this and some spacers
  LogInfo << "Number of samples in waveform: " << nSamples << std::endl;
  std::vector<std::vector<std::pair<int,double>>> h_waveforms(nChannels, std::vector<std::pair<int,double>>(nSamples, std::make_pair(0,0)));

  std::vector<std::vector<std::pair<int,double>>> h_waveforms_saturated(nChannels, std::vector<std::pair<int,double>>(nSamples, std::make_pair(0,0)));

  // average waveforms per channel
  std::vector <TGraphErrors*> g_aveWf;
  g_aveWf.reserve(256);
  for (int i = 0; i < 256; i++){
    g_aveWf[i] = new TGraphErrors();
    g_aveWf[i]->SetTitle(Form("aveWf_Ch%i", i));
    g_aveWf[i]->SetMarkerStyle(22);
    g_aveWf[i]->SetMarkerSize(0.6);
    g_aveWf[i]->SetMarkerColor(2);
    // set y limits between -0.1 and 1.2
    g_aveWf[i]->SetMinimum(-0.02);
    g_aveWf[i]->SetMaximum(0.22);
    // put labels
    g_aveWf[i]->GetXaxis()->SetTitle("Sample");
    g_aveWf[i]->GetYaxis()->SetTitle("Amplitude [V]");
    // g_aveWf[i]->SetStats(false);
    // g_aveWf[i]->GetXaxis()->SetTickLength(0);
    // g_aveWf[i]->GetXaxis()->SetLabelOffset(999);
    // g_aveWf[i]->GetYaxis()->SetTickLength(0);
  }

  // average waveforms per channel, when saturating
  std::vector <TGraphErrors*> g_aveWf_sat;
  g_aveWf_sat.reserve(256);
  for (int i = 0; i < 256; i++){
    g_aveWf_sat[i] = new TGraphErrors();
    g_aveWf_sat[i]->SetTitle(Form("aveWf_sat_Ch%i", i));
    g_aveWf_sat[i]->SetName(Form("aveWf_sat_Ch%i", i));
    g_aveWf_sat[i]->SetMarkerStyle(22);
    g_aveWf_sat[i]->SetMarkerSize(0.6);
    g_aveWf_sat[i]->SetMarkerColor(2);
    // set y limits between -0.1 and 1.2
    g_aveWf_sat[i]->SetMinimum(-0.02);
    g_aveWf_sat[i]->SetMaximum(1.2);
    // put labels
    g_aveWf_sat[i]->GetXaxis()->SetTitle("Sample");
    g_aveWf_sat[i]->GetYaxis()->SetTitle("Amplitude [V]");
    // g_aveWf[i]->SetStats(false);
    // g_aveWf[i]->GetXaxis()->SetTickLength(0);
    // g_aveWf[i]->GetXaxis()->SetLabelOffset(999);
    // g_aveWf[i]->GetYaxis()->SetTickLength(0);
  }


  TH1F *h_signalType = new TH1F("h_signalType", Form("Signal Type, run%i", thisRun.GetRunNumber()), 4, -0.5, 3.5);
  h_signalType->GetXaxis()->SetTitle("Signal Type");
  h_signalType->SetMinimum(0);
  hist_list->Add(h_signalType);

  // create a vector of 6 histograms called h_signalType_plane, one for each plane
  std::vector <TH1F*> h_signalType_plane;
  h_signalType_plane.reserve(6);
  for (int i = 0; i < 6; i++){
    h_signalType_plane[i] = new TH1F(Form("h_signalType_plane%s", PlaneLabels.at(i).c_str()), Form("Signal Type, run%i, plane%s", thisRun.GetRunNumber(), PlaneLabels.at(i).c_str()), 4, -0.5, 3.5);
    h_signalType_plane[i]->GetXaxis()->SetTitle("Sygnal Type");
    h_signalType_plane[i]->SetMinimum(0);
    hist_list->Add(h_signalType_plane[i]);
  }

  TH1F *h_hitsPerMidasEvent = new TH1F("h_hitsPerMidasEvent", Form("HitsperMidasEvent, run%i",thisRun.GetRunNumber()), 50, -0.5, 49.5);
  h_hitsPerMidasEvent->GetXaxis()->SetTitle("#hits per Midas Event");
  h_hitsPerMidasEvent->SetMinimum(0);
  hist_list->Add(h_hitsPerMidasEvent);

  TH1F *h_saturated_NTotHits = new TH1F("h_saturated_NTotHits", Form("Detector occupancy for a saturated Hit , run%i", thisRun.GetRunNumber()), 50, -0.5, 49.5);
  h_saturated_NTotHits->GetXaxis()->SetTitle("Channel");
  h_saturated_NTotHits->SetMinimum(0);
  h_saturated_NTotHits->SetFillColor(kRed);
  h_saturated_NTotHits->SetLineColor(kRed);
  h_saturated_NTotHits->SetFillStyle(3004);
  hist_list->Add(h_saturated_NTotHits);

  TH1F *h_saturated_signaltype = new TH1F("h_saturated_signaltype", Form("Signal type for a saturated Hit , run%i", thisRun.GetRunNumber()), 4, -0.5, 3.5);
  h_saturated_signaltype->GetXaxis()->SetTitle("Channel");
  h_saturated_signaltype->SetMinimum(0);
  h_saturated_signaltype->SetFillColor(kRed);
  h_saturated_signaltype->SetLineColor(kRed);
  h_saturated_signaltype->SetFillStyle(3004);
  hist_list->Add(h_saturated_signaltype);

  TH1F *h_fitpar0_fitSuccess = new TH1F("h_fitpar0_fitSuccess", Form("fitpar0_fitSuccess, run%i", thisRun.GetRunNumber()), 100, 0, 1.2);
  h_fitpar0_fitSuccess->GetXaxis()->SetTitle("FitParam0 (peak Amp. [V])");
  h_fitpar0_fitSuccess->SetMinimum(0);
  h_fitpar0_fitSuccess->SetLineColor(2);
  hist_list->Add(h_fitpar0_fitSuccess);

  TH1F *h_fitpar0_fitFail = new TH1F("h_fitpar0_fitFail", Form("fitpar0_fitFail, run%i", thisRun.GetRunNumber()), 100, 0, 1.2);
  h_fitpar0_fitFail->GetXaxis()->SetTitle("FitParam0 (peak Amp. [V])");
  h_fitpar0_fitFail->SetMinimum(0);
  h_fitpar0_fitFail->SetLineColor(4);
  hist_list->Add(h_fitpar0_fitFail);

  //par 1 
  TH1F *h_fitpar1_fitSuccess = new TH1F("h_fitpar1_fitSuccess", Form("fitpar1_fitSuccess, run%i", thisRun.GetRunNumber()), 63, 0, 63);
  h_fitpar1_fitSuccess->GetXaxis()->SetTitle("FitPar1 (sample peak [a.u.])");
  h_fitpar1_fitSuccess->SetMinimum(0);
  h_fitpar1_fitSuccess->SetLineColor(2);
  hist_list->Add(h_fitpar1_fitSuccess);

  TH1F *h_fitpar1_fitFail = new TH1F("h_fitpar1_fitFail", Form("fitpar1_fitFail, run%i", thisRun.GetRunNumber()), 63, 0, 63);
  h_fitpar1_fitFail->GetXaxis()->SetTitle("FitPar1 (sample peak [a.u.])");
  h_fitpar1_fitFail->SetMinimum(0);
  h_fitpar1_fitFail->SetLineColor(4);
  hist_list->Add(h_fitpar1_fitFail);

  //par 2
  TH1F *h_fitpar2_fitSuccess = new TH1F("h_fitpar2_fitSuccess", Form("fitpar2_fitSuccess, run%i", thisRun.GetRunNumber()), 100, 0, 2);
  h_fitpar2_fitSuccess->GetXaxis()->SetTitle("FitParam2 (widthDep1 [ns])");
  h_fitpar2_fitSuccess->SetMinimum(0);
  h_fitpar2_fitSuccess->SetLineColor(2);
  hist_list->Add(h_fitpar2_fitSuccess);

  TH1F *h_fitpar2_fitFail = new TH1F("h_fitpar2_fitFail", Form("fitpar2_fitFail, run%i", thisRun.GetRunNumber()), 100, 0, 2);
  h_fitpar2_fitFail->GetXaxis()->SetTitle("FitParam2 (widthDep1 [ns])");
  h_fitpar2_fitFail->SetMinimum(0);
  h_fitpar2_fitFail->SetLineColor(4);
  hist_list->Add(h_fitpar2_fitFail);

  //par 3
  TH1F *h_fitpar3_fitSuccess = new TH1F("h_fitpar3_fitSuccess", Form("fitpar3_fitSuccess, run%i", thisRun.GetRunNumber()), 100, 0, 50);
  h_fitpar3_fitSuccess->GetXaxis()->SetTitle("FitParam3 (width [ns])");
  h_fitpar3_fitSuccess->SetMinimum(0);
  h_fitpar3_fitSuccess->SetLineColor(2);
  hist_list->Add(h_fitpar3_fitSuccess);

  TH1F *h_fitpar3_fitFail = new TH1F("h_fitpar3_fitFail", Form("fitpar3_fitFail, run%i", thisRun.GetRunNumber()), 100, 0, 50);
  h_fitpar3_fitFail->GetXaxis()->SetTitle("FitParam3 (width [ns])");
  h_fitpar3_fitFail->SetMinimum(0);
  h_fitpar3_fitFail->SetLineColor(4);
  hist_list->Add(h_fitpar3_fitFail);

  //par 4
  TH1F *h_fitpar4_fitSuccess = new TH1F("h_fitpar4_fitSuccess", Form("fitpar4_fitSuccess, run%i", thisRun.GetRunNumber()), 100, 0, 2);
  h_fitpar4_fitSuccess->GetXaxis()->SetTitle("FitParam4 (widthDep2 [ns])");
  h_fitpar4_fitSuccess->SetMinimum(0);
  h_fitpar4_fitSuccess->SetLineColor(2);
  hist_list->Add(h_fitpar4_fitSuccess);

  TH1F *h_fitpar4_fitFail = new TH1F("h_fitpar4_fitFail", Form("fitpar4_fitFail, run%i", thisRun.GetRunNumber()), 100, 0, 2);
  h_fitpar4_fitFail->GetXaxis()->SetTitle("FitParam4 (widthDep2 [ns])");
  h_fitpar4_fitFail->SetMinimum(0);
  h_fitpar4_fitFail->SetLineColor(4);
  hist_list->Add(h_fitpar4_fitFail);

  //par 5
  TH1F *h_fitpar5_fitSuccess = new TH1F("h_fitpar5_fitSuccess", Form("fitpar5_fitSuccess, run%i", thisRun.GetRunNumber()), 100, -0.02, 0.05);
  h_fitpar5_fitSuccess->GetXaxis()->SetTitle("FitParam5 (baseline [mV])");
  h_fitpar5_fitSuccess->SetMinimum(0);
  h_fitpar5_fitSuccess->SetLineColor(2);
  hist_list->Add(h_fitpar5_fitSuccess);

  TH1F *h_fitpar5_fitFail = new TH1F("h_fitpar5_fitFail", Form("fitpar5_fitFail, run%i", thisRun.GetRunNumber()), 100, -0.02, 0.05);
  h_fitpar5_fitFail->GetXaxis()->SetTitle("FitParam5 (baseline [mV])");
  h_fitpar5_fitFail->SetMinimum(0);
  h_fitpar5_fitFail->SetLineColor(4);
  hist_list->Add(h_fitpar5_fitFail);


  TH2F *h_fitCheck_fitSuccess = new TH2F("h_fitCheck_fitSuccess", Form("fitCheck_fitSuccess, run%i", thisRun.GetRunNumber()), 100, 0, 1.2, 63, 0, 63 );
  h_fitCheck_fitSuccess->GetXaxis()->SetTitle("FitAmplitude [V]");
  h_fitCheck_fitSuccess->GetYaxis()->SetTitle("Fit max amp sample [a.u.]");
  //h_fitCheck_fitSuccess->SetMinimum(0);
  h_fitCheck_fitSuccess->SetMarkerStyle(24);
  h_fitCheck_fitSuccess->SetMarkerColor(2);
  hist_list->Add(h_fitCheck_fitSuccess);


  TH2F *h_fitCheck_fitFail = new TH2F("h_fitCheck_fitFail", Form("fitCheck_fitFail, run%i", thisRun.GetRunNumber()), 100, 0, 1.2, 63, 0, 63 );
  h_fitCheck_fitFail->GetXaxis()->SetTitle("FitAmplitude [V]");
  h_fitCheck_fitFail->GetYaxis()->SetTitle("Fit max amp sample [a.u.]");
  h_fitCheck_fitFail->SetMarkerStyle(24);
  h_fitCheck_fitFail->SetMarkerColor(4);
  hist_list->Add(h_fitCheck_fitFail);

  TH2F *h_risingTime_vs_maxAmp = new TH2F("h_risingTime_vs_maxAmp", Form("risingTime_vs_maxAmp, run%i", thisRun.GetRunNumber()), 50, 0, 20, 50, -0.1, 1.1 );
  h_risingTime_vs_maxAmp->GetXaxis()->SetTitle("risingTime [ns]");
  h_risingTime_vs_maxAmp->GetYaxis()->SetTitle("Fit max amp sample [a.u.]");
  h_risingTime_vs_maxAmp->SetMarkerStyle(24);
  h_risingTime_vs_maxAmp->SetMarkerColor(4);
  hist_list->Add(h_risingTime_vs_maxAmp);


  TH2F *h_ToT_vs_maxAmp = new TH2F("h_ToT_vs_maxAmp", Form("ToT_vs_maxAmp, run%i", thisRun.GetRunNumber()), 100, 0, 50, 50, -0.1, 1.1 );
  h_ToT_vs_maxAmp->GetXaxis()->SetTitle("ToT [ns]");
  h_ToT_vs_maxAmp->GetYaxis()->SetTitle("Fit max amp sample [a.u.]");
  h_ToT_vs_maxAmp->SetMarkerStyle(24);
  h_ToT_vs_maxAmp->SetMarkerColor(4);
  hist_list->Add(h_ToT_vs_maxAmp);

  TH2F *h_risingTime_vs_ToT = new TH2F("h_risingTime_vs_ToT", Form("risingTime_vs_ToT, run%i", thisRun.GetRunNumber()), 50, 0, 20, 140, 0, 70);
  h_risingTime_vs_ToT->GetXaxis()->SetTitle("risingTime [ns]");
  h_risingTime_vs_ToT->GetYaxis()->SetTitle("ToT [ns]");
  h_risingTime_vs_ToT->SetMarkerStyle(24);
  h_risingTime_vs_ToT->SetMarkerColor(4);
  hist_list->Add(h_risingTime_vs_ToT);


  TH2F *h_risingTime_vs_maxAmp_sgType3 = new TH2F("h_risingTime_vs_maxAmp_sgType3", Form("risingTime_vs_maxAmp_sgType3, run%i", thisRun.GetRunNumber()), 50, 0, 20, 50, -0.1, 1.2 );
  h_risingTime_vs_maxAmp_sgType3->GetXaxis()->SetTitle("risingTime [ns]");
  h_risingTime_vs_maxAmp_sgType3->GetYaxis()->SetTitle("Max amp[V]");
  h_risingTime_vs_maxAmp_sgType3->SetMarkerStyle(24);
  h_risingTime_vs_maxAmp_sgType3->SetMarkerColor(4);
  hist_list->Add(h_risingTime_vs_maxAmp_sgType3);

  TH2F *h_risingTime_vs_ToT_sgType3 = new TH2F("h_risingTime_vs_ToT_sgType3", Form("risingTime_vs_ToT_sgType3, run%i", thisRun.GetRunNumber()), 50, 0, 20, 140, 0, 70);
  h_risingTime_vs_ToT_sgType3->GetXaxis()->SetTitle("risingTime [ns]");
  h_risingTime_vs_ToT_sgType3->GetYaxis()->SetTitle("ToT [ns]");
  h_risingTime_vs_ToT_sgType3->SetMarkerStyle(24);
  h_risingTime_vs_ToT_sgType3->SetMarkerColor(4);
  hist_list->Add(h_risingTime_vs_ToT_sgType3);


  TH2F *h_ToT_vs_maxAmp_sgType3 = new TH2F("h_ToT_vs_maxAmp_sgType3", Form("ToT_vs_maxAmp_sgType3, run%i", thisRun.GetRunNumber()), 140, 0, 70, 50, -0.1, 1.2 );
  h_ToT_vs_maxAmp_sgType3->GetXaxis()->SetTitle("ToT [ns]");
  h_ToT_vs_maxAmp_sgType3->GetYaxis()->SetTitle("Max amp [V]");
  h_ToT_vs_maxAmp_sgType3->SetMarkerStyle(24);
  h_ToT_vs_maxAmp_sgType3->SetMarkerColor(4);
  hist_list->Add(h_ToT_vs_maxAmp_sgType3);


  TH1F *h_signalPosition_sgType3 = new TH1F("h_signalPosition_sgType3", Form("SignalPosition_sgType3, run%i", thisRun.GetRunNumber()), 50, -160, 160.);
  h_signalPosition_sgType3->GetXaxis()->SetTitle("Position [cm]");
  h_signalPosition_sgType3->SetMinimum(0);

  TH1F *h_channelsFiring_sgType3 = new TH1F("h_channelsFiring_sgType3", Form("ChannelsFiring_sgType3, run%i", thisRun.GetRunNumber()), 256, -0.5, 255.5);
  h_channelsFiring_sgType3->GetXaxis()->SetTitle("Channel");
  h_channelsFiring_sgType3->SetMinimum(0);

  TH1F *h_timeOfFlight_sgType3 = new TH1F("h_timeOfFlight_sgType3", Form("TimeOfFlight_sgType3, run%i", thisRun.GetRunNumber()), 56, -0.5, 55.5);
  h_timeOfFlight_sgType3->GetXaxis()->SetTitle("Time of Flight [ns]");
  h_timeOfFlight_sgType3->SetMinimum(0);

  TH1F *h_hitPeak_sgType3 = new TH1F("h_hitPeak_sgType3", Form("HitPeak, run%i", thisRun.GetRunNumber()), 200, -0.1,1.1 );
  h_hitPeak_sgType3->GetXaxis()->SetTitle("Peak [V]");
  h_hitPeak_sgType3->SetMinimum(0);


  // Creat Histogram for top_vs_planes
std::vector <TH2F*> h_top_vs_planes; // better static?
  h_planes.reserve(6);
  for (int i = 0; i < 6; i++){
    h_top_vs_planes[i] = new TH2F(Form("plane%s", PlaneLabels.at(i).c_str()), Form("plane%s", PlaneLabels.at(i).c_str()), 1, -110, 110, 20, -0.5, 19.5);
    h_top_vs_planes[i]->SetStats(false);
    // h_planes[i]->GetXaxis()->SetTickLength(0);
    // h_planes[i]->GetXaxis()->SetLabelOffset(999);
    h_top_vs_planes[i]->GetYaxis()->SetTickLength(0);
    h_top_vs_planes[i]->GetYaxis()->SetTitle("top");
    h_top_vs_planes[i]->GetXaxis()->SetTitle("Position [cm]");
    hist_list->Add(h_top_vs_planes[i]);
  }
 


  //////////////////////////////////////////////////////////////

  LogInfo << "Run number: " << thisRun.GetRunNumber() << std::endl;
  LogInfo << "Run address: " << thisRun.GetRunAddress() << std::endl;
  LogInfo << "Number of events: " << thisRun.GetRunEventsList().size() << std::endl;

  // leaving this commented for the moment, when reading odb config 
  // from MIDAS, if we had a gate or not can be known from there TODO

  // if (thisRun.GetRunGate()){
  //   std::cout << " basic check : size of the gate list " << thisRun.GetRunGatesTimestamps().size() << std::endl;
  //   std::cout << " basic check : size of the Event list " << thisRun.GetRunEventsList().size() << std::endl;
  //   std::cout << " basic check : size of the spill number list " << thisRun.GetRunSpillNumberFromExtTrig().size() << std::endl;
  // }




  for (auto midashits: thisRun.GetRunHitsperMIDASEventList()){
    h_hitsPerMidasEvent->Fill(midashits);
    //std::cout << midashits << std::endl;
  }

  for (auto midastofevent: thisRun.GetSAMPICEventsperMIDASList()){
    h_hitsPerMidasEvent->Fill(midastofevent);
    std::cout << midastofevent << std::endl;
  }


  int event_counter = 0;
  for (auto  eventit : thisRun.GetRunEventsList()) {
    // LogInfo << "enter event loop" << std::endl;
    for (auto  signalit : eventit.GetEventSignalsList()) {
      // if (event_counter % 100 == 0) {
      //   LogInfo << "Reading through Events, currently at " << event_counter / (double)(thisRun.RunEventsList.size()) * 100 << " %\r";
      //   LogInfo << std::flush;
      // }
      
      int thisSignalType = signalit.GetSignalType();

      bool has_hitMin = false, has_hitMax = false; 
      TofHit thisHitMin;
      TofHit thisHitMax;
      if (thisSignalType == 1 || thisSignalType == 3) {
        has_hitMin = true;    
        thisHitMin = signalit.GetSignalHitMin();
      }
      if (thisSignalType == 2 || thisSignalType == 3) { 
        has_hitMax = true;
        thisHitMax = signalit.GetSignalHitMax();
      }

      h_signalType->Fill(thisSignalType);  
      // if has_hitMin, use the min. Otherwise, it will have hitMax. Avoiding double calls in this way.
      if (has_hitMin){ 
        h_signalType_plane[thisHitMin.GetHitPlane()]->Fill(thisSignalType);
      }
      else {
        h_signalType_plane[thisHitMax.GetHitPlane()]->Fill(thisSignalType);
      } 
      
      if (has_hitMin) {
        auto& thisHitMin = signalit.GetSignalHitMin();
        auto thisHitMinChannel = thisHitMin.GetHitDaqChannel();
        if (!has_hitMax) h_singleHitPeak->Fill(thisHitMin.GetHitPeak());
        h_channelsFiring->Fill(thisHitMinChannel);
        if (thisHitMin.GetHitIsSaturated()) h_saturatedHits->Fill(thisHitMinChannel);
        if (waveform_display&&thisSignalType == 3) thisHitMin.HitDisplayWaveform();

        h_baseline[thisHitMinChannel]->Fill(thisHitMin.GetHitBaseline());
        h_maxAmp[thisHitMinChannel]->Fill(thisHitMin.GetHitPeak());
        h_peakSample[thisHitMinChannel]->Fill(thisHitMin.GetHitPeakSample());
        h_integral[thisHitMinChannel]->Fill(thisHitMin.GetHitVoltageIntegral());

        double HitMin_risingTime = thisHitMin.GetHitCfTime_0p9() - thisHitMin.GetHitCfTime_0p1();
        h_risingTime[thisHitMinChannel]->Fill(HitMin_risingTime);
        h_risingTime_vs_maxAmp->Fill(HitMin_risingTime, thisHitMin.GetHitPeak());
        h_risingTime_vs_ToT->Fill(HitMin_risingTime, thisHitMin.GetHitTotValue());
        h_ToT_vs_maxAmp->Fill(thisHitMin.GetHitTotValue(), thisHitMin.GetHitPeak());

        if (thisSignalType == 3){
          h_risingTime_vs_maxAmp_sgType3->Fill(HitMin_risingTime, thisHitMin.GetHitPeak());
          h_risingTime_vs_ToT_sgType3->Fill(HitMin_risingTime, thisHitMin.GetHitTotValue());
          h_ToT_vs_maxAmp_sgType3->Fill(thisHitMin.GetHitTotValue(), thisHitMin.GetHitPeak());
          h_signalPosition_sgType3->Fill(signalit.GetSignalPosition());
          h_channelsFiring_sgType3->Fill(thisHitMinChannel);
        }

        // Draw fit parameters
        if (thisHitMin.GetHitFitSuccess()) {
          h_fitpar0_fitSuccess->Fill(thisHitMin.GetHitFitFunction().GetParameter(0));
          h_fitpar1_fitSuccess->Fill(thisHitMin.GetHitFitFunction().GetParameter(1));
          h_fitpar2_fitSuccess->Fill(thisHitMin.GetHitFitFunction().GetParameter(2));
          h_fitpar3_fitSuccess->Fill(thisHitMin.GetHitFitFunction().GetParameter(3));
          h_fitpar4_fitSuccess->Fill(thisHitMin.GetHitFitFunction().GetParameter(4));
          h_fitpar5_fitSuccess->Fill(thisHitMin.GetHitFitFunction().GetParameter(5));
          h_fitCheck_fitSuccess->Fill(thisHitMin.GetHitFitFunction().GetParameter(0), thisHitMin.GetHitFitFunction().GetParameter(1));
        }

        if (!thisHitMin.GetHitFitSuccess()) {
          h_fitpar0_fitFail->Fill(thisHitMin.GetHitFitFunction().GetParameter(0));
          h_fitpar1_fitFail->Fill(thisHitMin.GetHitFitFunction().GetParameter(1));
          h_fitpar2_fitFail->Fill(thisHitMin.GetHitFitFunction().GetParameter(2));
          h_fitpar3_fitFail->Fill(thisHitMin.GetHitFitFunction().GetParameter(3));
          h_fitpar4_fitFail->Fill(thisHitMin.GetHitFitFunction().GetParameter(4));
          h_fitpar5_fitFail->Fill(thisHitMin.GetHitFitFunction().GetParameter(5));
          h_fitCheck_fitFail->Fill(thisHitMin.GetHitFitFunction().GetParameter(0), thisHitMin.GetHitFitFunction().GetParameter(1));
        }

        for (int i = 0; i < nSamples; i++) {
          h_waveforms.at(thisHitMinChannel).at(i).second += thisHitMin.GetHitWaveform()[i];
          h_waveforms.at(thisHitMinChannel).at(i).first++;
          if (thisHitMin.GetHitIsSaturated()) {
            h_waveforms_saturated.at(thisHitMinChannel).at(i).second += thisHitMin.GetHitWaveform()[i];
            h_waveforms_saturated.at(thisHitMinChannel).at(i).first++;
          }
        }
      }

      if (has_hitMax) {
        auto& thisHitMax = signalit.GetSignalHitMax();
        auto thisHitMaxChannel = thisHitMax.GetHitDaqChannel();
        if (!has_hitMin) h_singleHitPeak->Fill(thisHitMax.GetHitPeak());
        h_channelsFiring->Fill(thisHitMaxChannel);
        if (thisHitMax.GetHitIsSaturated()) {
          h_saturatedHits->Fill(thisHitMaxChannel);
          h_saturated_NTotHits->Fill(eventit.GetEventHitsList().size());
          h_saturated_signaltype->Fill(signalit.GetSignalType());
        }
        if (waveform_display&&thisSignalType == 3) thisHitMax.HitDisplayWaveform();

        h_baseline[thisHitMaxChannel]->Fill(thisHitMax.GetHitBaseline());
        h_maxAmp[thisHitMaxChannel]->Fill(thisHitMax.GetHitPeak());
        h_peakSample[thisHitMaxChannel]->Fill(thisHitMax.GetHitPeakSample());
        h_integral[thisHitMaxChannel]->Fill(thisHitMax.GetHitVoltageIntegral());

        double HitMax_risingTime = thisHitMax.GetHitCfTime_0p9() - thisHitMax.GetHitCfTime_0p1();
        h_risingTime[thisHitMaxChannel]->Fill(HitMax_risingTime);
        h_risingTime_vs_maxAmp->Fill(HitMax_risingTime, thisHitMax.GetHitPeak());
        h_risingTime_vs_ToT->Fill(HitMax_risingTime, thisHitMax.GetHitTotValue());
        h_ToT_vs_maxAmp->Fill(thisHitMax.GetHitTotValue(), thisHitMax.GetHitPeak());

        if(thisSignalType == 3){
          h_risingTime_vs_maxAmp_sgType3->Fill(HitMax_risingTime, thisHitMax.GetHitPeak());
          h_risingTime_vs_ToT_sgType3->Fill(HitMax_risingTime, thisHitMax.GetHitTotValue());
          h_ToT_vs_maxAmp_sgType3->Fill(thisHitMax.GetHitTotValue(), thisHitMax.GetHitPeak());
          h_signalPosition_sgType3->Fill(signalit.GetSignalPosition());
          h_channelsFiring_sgType3->Fill(thisHitMaxChannel);
        }
        // Draw fit parameters
        if (thisHitMax.GetHitFitSuccess()) {
          h_fitpar0_fitSuccess->Fill(thisHitMax.GetHitFitFunction().GetParameter(0));
          h_fitpar1_fitSuccess->Fill(thisHitMax.GetHitFitFunction().GetParameter(1));
          h_fitpar2_fitSuccess->Fill(thisHitMax.GetHitFitFunction().GetParameter(2));
          h_fitpar3_fitSuccess->Fill(thisHitMax.GetHitFitFunction().GetParameter(3));
          h_fitpar4_fitSuccess->Fill(thisHitMax.GetHitFitFunction().GetParameter(4));
          h_fitpar5_fitSuccess->Fill(thisHitMax.GetHitFitFunction().GetParameter(5));
          h_fitCheck_fitSuccess->Fill(thisHitMax.GetHitFitFunction().GetParameter(0), thisHitMax.GetHitFitFunction().GetParameter(1));
        }

        if (!thisHitMax.GetHitFitSuccess()) {
          h_fitpar0_fitFail->Fill(thisHitMax.GetHitFitFunction().GetParameter(0));
          h_fitpar1_fitFail->Fill(thisHitMax.GetHitFitFunction().GetParameter(1));
          h_fitpar2_fitFail->Fill(thisHitMax.GetHitFitFunction().GetParameter(2));
          h_fitpar3_fitFail->Fill(thisHitMax.GetHitFitFunction().GetParameter(3));
          h_fitpar4_fitFail->Fill(thisHitMax.GetHitFitFunction().GetParameter(4));
          h_fitpar5_fitFail->Fill(thisHitMax.GetHitFitFunction().GetParameter(5));
          h_fitCheck_fitFail->Fill(thisHitMax.GetHitFitFunction().GetParameter(0), thisHitMax.GetHitFitFunction().GetParameter(1));
        }

        for (int i = 0; i < nSamples; i++) {
          h_waveforms.at(thisHitMaxChannel).at(i).second += thisHitMax.GetHitWaveform()[i];
          h_waveforms.at(thisHitMaxChannel).at(i).first++;
          if (thisHitMax.GetHitIsSaturated()) {
              h_waveforms_saturated.at(thisHitMaxChannel).at(i).second += thisHitMax.GetHitWaveform()[i];
              h_waveforms_saturated.at(thisHitMaxChannel).at(i).first++;
          }
        }
      }

      if (has_hitMin && has_hitMax) {
        auto & thisHitMin = signalit.GetSignalHitMin();
        auto & thisHitMax = signalit.GetSignalHitMax();
        auto thisHitMinChannel = thisHitMin.GetHitDaqChannel();
        auto thisHitMaxChannel = thisHitMax.GetHitDaqChannel();
        h_signalPosition->Fill(signalit.GetSignalPosition());
        h_signalPosition_plane[thisHitMin.GetHitPlane()]->Fill(signalit.GetSignalPosition());
        
        //fill h_topplane_plane
       h_top_vs_planes[thisHitMin.GetHitPlane()]->Fill(signalit.GetSignalPosition(),signalit.GetSignalPosition());

        h_signalBar->Fill(thisHitMin.GetHitBar());
        h_signalBar_plane[thisHitMin.GetHitPlane()]->Fill(thisHitMin.GetHitBar());
        h_signalPlane->Fill(thisHitMin.GetHitPlane());
        h_hitPeak->Fill(thisHitMin.GetHitPeak());
        h_hitPeak->Fill(thisHitMax.GetHitPeak());
        h_hitPeak_plane[thisHitMin.GetHitPlane()]->Fill(thisHitMin.GetHitPeak());
        h_hitPeak_plane[thisHitMax.GetHitPlane()]->Fill(thisHitMax.GetHitPeak());
        h_channelsFiring_plane[thisHitMin.GetHitPlane()]->Fill(signalit.GetSignalHitMin().GetHitChannelOnPlane());
        h_channelsFiring_plane[thisHitMax.GetHitPlane()]->Fill(signalit.GetSignalHitMax().GetHitChannelOnPlane());
        if (thisHitMin.GetHitIsSaturated()){
          h_saturatedHits->Fill(thisHitMinChannel);
          h_saturatedHits_plane[thisHitMin.GetHitPlane()]->Fill(thisHitMinChannel);
          if (thisHitMax.GetHitIsSaturated()) {
            h_saturatedOtherEdge->Fill(thisHitMaxChannel);
            h_saturatedOtherEdge_plane[thisHitMax.GetHitPlane()]->Fill(thisHitMaxChannel);
          }
        }
        if (thisHitMax.GetHitIsSaturated()){
          h_saturatedHits->Fill(thisHitMaxChannel);
          h_saturatedHits_plane[thisHitMax.GetHitPlane()]->Fill(thisHitMaxChannel);
          if (thisHitMin.GetHitIsSaturated()) {
            h_saturatedOtherEdge->Fill(thisHitMinChannel);
            h_saturatedOtherEdge_plane[thisHitMin.GetHitPlane()]->Fill(thisHitMinChannel);
          }
        }
        h_planes[thisHitMin.GetHitPlane()]->Fill(signalit.GetSignalPosition(), thisHitMin.GetHitBar()); 
        g_hits[thisHitMin.GetHitPlane()]->SetPoint(g_hits[thisHitMin.GetHitPlane()]->GetN(), signalit.GetSignalPosition(), thisHitMin.GetHitBar());
      }
    }

    if (eventit.GetEventTimeOfFlight() != 0) {
      // LogInfo << "Event has time of flight: " << eventit.GetEventTimeOfFlight() << std::endl;
      h_timeOfFlight->Fill(eventit.GetEventTimeOfFlight());
      // this is actually the same as above, since the TOF is non-zero only for sgType3
      // could be removed, but I leave it for now TODO
      h_timeOfFlight_sgType3->Fill(eventit.GetEventTimeOfFlight()); 
    }

    event_counter++;
  }

  LogInfo << "\nNow plotting histograms" << std::endl;

  TCanvas *c_allHits = new TCanvas("c_allHits", "allHits", 900, 900);
  c_allHits->Divide(2,2);
  c_allHits->cd(1);
  h_channelsFiring->Draw("HIST");
  c_allHits->cd(2);
  h_hitPeak->Draw("HIST");
  c_allHits->SaveAs(Form("%srun%i_allHits.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_allHits->SaveAs(Form("%srun%i_allHits.pdf", output_directory.c_str(), thisRun.GetRunNumber()));

  TCanvas *c_allSignals = new TCanvas("c_allSignals", "allSignals", 900, 900);
  c_allSignals->Divide(3,2);
  c_allSignals->cd(1);
  h_signalBar->Draw("HIST");
  c_allSignals->cd(2);
  h_signalPlane->Draw("HIST");
  c_allSignals->cd(3);
  h_signalPosition->Draw("HIST");
  c_allSignals->cd(4);
  h_signalType->Draw("HIST");
  c_allSignals->cd(5);
  // empty
  c_allSignals->cd(6);
  h_timeOfFlight->Draw("HIST");
  c_allSignals->SaveAs(Form("%srun%i_allSignals.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_allSignals->SaveAs(Form("%srun%i_allSignals.pdf", output_directory.c_str(), thisRun.GetRunNumber()));
  
  // create vector of 6 canvas to host the 6 planes. Divide each canvas in 6 pads, for different plots
  std::vector<TCanvas*> c_planeSignals;
  c_planeSignals.reserve(6);
  for (int i = 0; i < 6; i++){
    c_planeSignals[i] = new TCanvas(Form("c_plane%iSignals", i), Form("plane%iSignals", i), 900, 900);
    c_planeSignals[i]->Divide(3,2);
    c_planeSignals[i]->cd(1);
    h_signalBar_plane[i]->Draw("HIST");
    c_planeSignals[i]->cd(2);
    h_signalPosition_plane[i]->Draw("HIST");
    c_planeSignals[i]->cd(3);
    h_hitPeak_plane[i]->Draw("HIST");
    c_planeSignals[i]->cd(4);
    h_channelsFiring_plane[i]->Draw("HIST");
    c_planeSignals[i]->cd(5);
    h_signalType_plane[i]->Draw("HIST");
    c_planeSignals[i]->cd(6);
    h_saturatedHits_plane[i]->Draw("HIST");

    c_planeSignals[i]->SaveAs(Form("%srun%i_plane%i.C", output_directory.c_str(), thisRun.GetRunNumber(), i));
    c_planeSignals[i]->SaveAs(Form("%srun%i_plane%i.pdf", output_directory.c_str(), thisRun.GetRunNumber(), i));
  }
// draw top vs planes
std::vector<TCanvas*> c_top_vs_planes;
  c_top_vs_planes.reserve(6);
  for (int i = 0; i < 6; i++){
    c_top_vs_planes[i] = new TCanvas(Form("c_top_plane%i", i), Form("top_plane%i", i), 900, 900);
    c_top_vs_planes[i]->Divide(3,2);
    c_top_vs_planes[i]->cd(1);
    h_top_vs_planes[i]->Draw("HIST");
    

    c_top_vs_planes[i]->SaveAs(Form("%srun%i_plane%i.C", output_directory.c_str(), thisRun.GetRunNumber(), i));
    c_top_vs_planes[i]->SaveAs(Form("%srun%i_plane%i.pdf", output_directory.c_str(), thisRun.GetRunNumber(), i));
  }
  // plot bad signals in some way
  TCanvas *c_badSignals = new TCanvas("c_badSignals", "'Bad' Signals", 900, 900);
  c_badSignals->Divide(2,2);
  c_badSignals->cd(1);
  h_singleHitPeak->Draw("HIST");
  c_badSignals->cd(2);
  h_channelsFiring->Draw("HIST");
  h_saturatedHits->Draw("SAMES");
  c_badSignals->cd(3);
  h_saturatedHits->Draw("HIST");
  h_saturatedOtherEdge->Draw("SAMES");
  c_badSignals->SaveAs(Form("%srun%i_badSignals.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_badSignals->SaveAs(Form("%srun%i_badSignals.pdf", output_directory.c_str(), thisRun.GetRunNumber()));

  // plot event display, first adjust color palette
  float max_value_hist = 0.;
  for (int i = 0; i < 6; i++) {
    float this_hist_max = h_planes[i]->GetMaximum();
    if (this_hist_max > max_value_hist) {
      max_value_hist = this_hist_max;
    }
  }
  // Create a color palette based on the maximum value
  int numColors = 100; // Adjust the number of colors as needed
  TColor::InitializeColors(); // Initialize the color palette
  gStyle->SetNumberContours(numColors); // Set the number of color levels
  for (int i = 0; i < 6; i++) {
    h_planes[i]->SetMaximum(max_value_hist);
    h_planes[i]->SetMinimum(0.);
    h_planes[i]->SetContour(numColors); // Set the number of contours for color mapping
  }

  TCanvas* c_planes = new TCanvas("c_eventDisplay", Form("EventDisplay, run%i", thisRun.GetRunNumber()), 600, 400);
  c_planes->Divide(3,2);
  c_planes->cd(1);
  h_planes[kTopModule]->Draw("COLZ");
  if (g_hits[kTopModule]->GetN() > 0) g_hits[kTopModule]->Draw("Psame");
  c_planes->cd(6);
  h_planes[kNorthModule]->Draw("COLZ");
  if (g_hits[kNorthModule]->GetN() > 0) g_hits[kNorthModule]->Draw("Psame");
  c_planes->cd(2);
  h_planes[kUpstreamModule]->Draw("COLZ");
  if (g_hits[kUpstreamModule]->GetN() > 0) g_hits[kUpstreamModule]->Draw("Psame");
  c_planes->cd(3);
  h_planes[kSouthModule]->Draw("COLZ");
  if (g_hits[kSouthModule]->GetN() > 0) g_hits[kSouthModule]->Draw("Psame");
  c_planes->cd(4);
  h_planes[kBottomModule]->Draw("COLZ");
  if (g_hits[kBottomModule]->GetN() > 0) g_hits[kBottomModule]->Draw("Psame");
  c_planes->cd(5);
  h_planes[kDownstreamModule]->Draw("COLZ");
  if (g_hits[kDownstreamModule]->GetN() > 0) g_hits[kDownstreamModule]->Draw("Psame");
  c_planes->SaveAs(Form("%srun%i_eventDisplay.C", output_directory.c_str(), thisRun.GetRunNumber()));
  // horizontal lines for better visualization, root file doesnt open if I add them before
  for (int i = 0; i < 6; i++) {  
    for (int j = 1; j <= 20; j++) {
      TLine* line = new TLine(-110, j - 0.5, 110, j - 0.5);
      h_planes[i]->GetListOfFunctions()->Add(line);
    }
  }
  c_planes->SaveAs(Form("%srun%i_eventDisplay.pdf", output_directory.c_str(), thisRun.GetRunNumber()));

  // monitoring plots
  TCanvas *c_monitoring = new TCanvas("c_monitoring", Form("Monitoring, run %i", thisRun.GetRunNumber()), 900, 900);
  c_monitoring->Divide(3,2);
  c_monitoring->cd(1);
  TGraph *g_baseline = new TGraph();
  for (int i = 0; i < nChannels; i++) {
    g_baseline->SetPoint(i, i, h_baseline[i]->GetMean());
  }
  g_baseline->SetTitle(Form("Baseline, run%i", thisRun.GetRunNumber()));
  g_baseline->SetMarkerStyle(2);
  g_baseline->SetMarkerSize(1);
  g_baseline->SetMarkerColor(kRed);
  // set labels
  g_baseline->GetXaxis()->SetTitle("Channel");
  g_baseline->GetYaxis()->SetTitle("Baseline [V]");
  g_baseline->Draw("AP");

  c_monitoring->cd(2);
  TGraph *g_maxAmp = new TGraph();
  for (int i = 0; i < nChannels; i++) {
    g_maxAmp->SetPoint(i, i, h_maxAmp[i]->GetMean());
  }
  g_maxAmp->SetTitle(Form("Max Amplitude, run%i", thisRun.GetRunNumber()));
  g_maxAmp->SetMarkerStyle(2);
  g_maxAmp->SetMarkerSize(1);
  g_maxAmp->SetMarkerColor(kRed);
  // labels
  g_maxAmp->GetXaxis()->SetTitle("Channel");
  g_maxAmp->GetYaxis()->SetTitle("Peak Amplitude [V]");
  g_maxAmp->Draw("AP");
  c_monitoring->cd(3);
  TGraph *g_peakSample = new TGraph();
  for (int i = 0; i < nChannels; i++) {
    g_peakSample->SetPoint(i, i, h_peakSample[i]->GetMean());
  }
  g_peakSample->SetTitle(Form("Peak Sample, run%i", thisRun.GetRunNumber()));
  g_peakSample->SetMarkerStyle(2);
  g_peakSample->SetMarkerSize(1);
  g_peakSample->SetMarkerColor(kRed);
  // labels
  g_peakSample->GetXaxis()->SetTitle("Channel");
  g_peakSample->GetYaxis()->SetTitle("Peak Sample");
  g_peakSample->Draw("AP");
  c_monitoring->cd(4);
  TGraph *g_risingTime = new TGraph();
  for (int i = 0; i < nChannels; i++) {
    g_risingTime->SetPoint(i, i, h_risingTime[i]->GetMean());
  }
  g_risingTime->SetTitle(Form("Rising Time, run%i", thisRun.GetRunNumber()));
  g_risingTime->SetMarkerStyle(2);
  g_risingTime->SetMarkerSize(1);
  g_risingTime->SetMarkerColor(kRed);
  // labels
  g_risingTime->GetXaxis()->SetTitle("Channel");
  g_risingTime->GetYaxis()->SetTitle("Rising Time [ns]");
  g_risingTime->Draw("AP");
  c_monitoring->cd(5);
  TGraph *g_integral = new TGraph();
  for (int i = 0; i < nChannels; i++) {
    g_integral->SetPoint(i, i, h_integral[i]->GetMean()*thisRun.GetRunSamplingTime());
  }
  g_integral->SetTitle(Form ("Integral, run%i", thisRun.GetRunNumber()));
  g_integral->SetMarkerStyle(2);
  g_integral->SetMarkerSize(1);
  g_integral->SetMarkerColor(kRed);
  // labels
  g_integral->GetXaxis()->SetTitle("Channel");
  g_integral->GetYaxis()->SetTitle("Integral [V ns]");
  g_integral->Draw("AP");
  // 6 is empty for now
  c_monitoring->SaveAs(Form("%srun%i_monitoring.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_monitoring->SaveAs(Form("%srun%i_monitoring.pdf", output_directory.c_str(), thisRun.GetRunNumber()));

  // one canvas 4x4 with the g_aveWf
  // fill g_aveWf with the averages of the h_waveforms
  for (int i = 0; i < nChannels; i++){
    for (int j = 0; j < nSamples; j++){
      g_aveWf[i]->SetPoint(j, j, h_waveforms[i][j].second/double(h_waveforms[i][j].first));
      g_aveWf_sat[i]->SetPoint(j, j, h_waveforms_saturated[i][j].second/double(h_waveforms_saturated[i][j].first));
    }
  }

  TCanvas *c_aveWf = new TCanvas("c_aveWf", Form("Average Waveforms, run %i", thisRun.GetRunNumber()), 600, 700);
  c_aveWf->Divide(4,4);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      int canvas_number = (i)*4 + j+1;
      c_aveWf->cd(canvas_number);
      // set title as FEBx, SAMPICx
      g_aveWf[canvas_number]->SetTitle(Form("FEB%i, SAMPIC%i", i, j));
      g_aveWf[canvas_number]->Draw("AL");
      for (int k = 1; k < 16; k++) g_aveWf[canvas_number + k]->Draw("L");
    }
  }
  c_aveWf->SaveAs(Form("%srun%i_aveWf.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_aveWf->SaveAs(Form("%srun%i_aveWf.pdf", output_directory.c_str(), thisRun.GetRunNumber()));

  TCanvas *c_aveWf_sat = new TCanvas("c_aveWf_sat", Form("Average Waveforms saturated, run %i", thisRun.GetRunNumber()), 600, 700);
  c_aveWf_sat->Divide(4,4);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      int canvas_number = (i)*4 + j+1;
      c_aveWf_sat->cd(canvas_number);
      // set title as FEBx, SAMPICx
      g_aveWf_sat[canvas_number]->SetTitle(Form("FEB%i, SAMPIC%i", i, j));
      g_aveWf_sat[canvas_number]->Draw("AL");
      for (int k = 1; k < 16; k++) g_aveWf_sat[canvas_number + k]->Draw("L");
    }
  }
  c_aveWf_sat->SaveAs(Form("%srun%i_aveWf_sat.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_aveWf_sat->SaveAs(Form("%srun%i_aveWf_sat.pdf", output_directory.c_str(), thisRun.GetRunNumber()));


  if (thisRun.GetRunNumber() >= 1175) {
    TH1F *h_cell0times = new TH1F("h_cell0times", Form("Timestamps, run%i", thisRun.GetRunNumber()), 10000, 0, 5e7);
    h_cell0times->GetXaxis()->SetTitle("Timestamp [ns]");
    h_cell0times->SetMinimum(0);
    h_cell0times->SetFillColor(kBlue);
    for (auto hitit : thisRun.GetRunOrderedHitsList()) {
      h_cell0times->Fill(hitit.GetHitCell0Time());
    }
    double gate_size = 1e5; // take from somewhere else
    TCanvas *c_timestamps = new TCanvas("c_timestamps", Form("Timestamps of hits and gates, run%i", thisRun.GetRunNumber()), 900, 900);
    c_timestamps->cd();
    TLegend *leg_timestamps = new TLegend();
    leg_timestamps->AddEntry(h_cell0times, "Hits", "l");
    h_cell0times->Draw("HIST");

    // add vertical lines in the positions where the GateTimeStamps are, start and end
    bool appo_first_line = true;
    for (auto gateit : thisRun.GetRunGatesTimestamps()) {
      TLine* line_start = new TLine(gateit, 0, gateit, 1);
      TLine* line_stop = new TLine(gateit + gate_size, 0, gateit + gate_size, 1);
      line_start->SetLineColor(kRed);
      line_start->SetLineStyle(2);
      line_stop->SetLineColor(kRed);
      line_stop->SetLineStyle(3);
      line_start->Draw("SAMES");
      line_stop->Draw("SAMES");
      if (appo_first_line) leg_timestamps->AddEntry(line_start, "Gate", "l");
      appo_first_line = false;
    }
    leg_timestamps->Draw();
    c_timestamps->SaveAs(Form("%srun%i_cell0times.C", output_directory.c_str(), thisRun.GetRunNumber()));
    c_timestamps->SaveAs(Form("%srun%i_cell0times.pdf", output_directory.c_str(), thisRun.GetRunNumber()));
  }

  TCanvas *c_fitStudies = new TCanvas("c_fitStudies", Form("Fit studies, run %i", thisRun.GetRunNumber()), 900, 900);
  c_fitStudies->Divide(3, 2);
  c_fitStudies->cd(1);
  h_fitpar0_fitSuccess->Draw("HIST");
  h_fitpar0_fitFail->Draw("same");

  c_fitStudies->cd(2);
  h_fitpar1_fitSuccess->Draw("HIST");
  h_fitpar1_fitFail->Draw("same");

  c_fitStudies->cd(3);
  h_fitpar2_fitSuccess->Draw("HIST");
  h_fitpar2_fitFail->Draw("same");

  c_fitStudies->cd(4);
  h_fitpar3_fitSuccess->Draw("HIST");
  h_fitpar3_fitFail->Draw("same");

  c_fitStudies->cd(5);
  h_fitpar4_fitSuccess->Draw("HIST");
  h_fitpar4_fitFail->Draw("same");

  c_fitStudies->cd(6);
  h_fitpar5_fitSuccess->Draw("HIST");
  h_fitpar5_fitFail->Draw("same");
  c_fitStudies->SaveAs(Form("%srun%i_fitStudies.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_fitStudies->SaveAs(Form("%srun%i_fitStudies.pdf", output_directory.c_str(), thisRun.GetRunNumber()));

  TCanvas *c_fitStudies_2 = new TCanvas("c_fitStudies_2", Form("fitstudies, run %i", thisRun.GetRunNumber()), 600, 500);
  c_fitStudies_2->Divide(2, 1);
  c_fitStudies_2->cd(1);
  h_fitCheck_fitSuccess->Draw("P");
  h_fitCheck_fitFail->Draw("Psame");
  c_fitStudies_2->SaveAs(Form("%srun%i_fitStudies_2.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_fitStudies_2->SaveAs(Form("%srun%i_fitStudies_2.pdf", output_directory.c_str(), thisRun.GetRunNumber()));

  TCanvas *c_noisestudy = new TCanvas("c_noisestudy", Form("noise studies, run %i", thisRun.GetRunNumber()), 1200, 500);
  c_noisestudy->Divide(2, 1);
  c_noisestudy->cd(1);
  h_risingTime_vs_maxAmp->Draw("COLZ");

  c_noisestudy->cd(2);
  h_risingTime_vs_ToT->Draw("COLZ");
  c_noisestudy->SaveAs(Form("%srun%i_noisestudy.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_noisestudy->SaveAs(Form("%srun%i_noisestudy.pdf", output_directory.c_str(), thisRun.GetRunNumber()));

  TCanvas *c_sgType3 = new TCanvas("c_sgType3", Form("signalType3, run %i", thisRun.GetRunNumber()), 1000, 1200);
  c_sgType3->Divide(2, 3);
  c_sgType3->cd(1);
  h_channelsFiring_sgType3->Draw();

  c_sgType3->cd(2);
  h_signalPosition_sgType3->Draw();

  c_sgType3->cd(3);
  h_timeOfFlight_sgType3->Draw();

  c_sgType3->cd(4);
  h_risingTime_vs_maxAmp_sgType3->Draw("COLZ");

  c_sgType3->cd(5);
  h_risingTime_vs_ToT_sgType3->Draw("COLZ");
  
  c_sgType3->cd(6);
  h_ToT_vs_maxAmp_sgType3->Draw("COLZ");

  c_sgType3->SaveAs(Form("%srun%i_sgType3.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_sgType3->SaveAs(Form("%srun%i_sgType3.pdf", output_directory.c_str(), thisRun.GetRunNumber()));

  TCanvas *c_saturated = new TCanvas("c_saturated", Form("saturated wf studies, run %i", thisRun.GetRunNumber()), 900, 900);
  c_saturated->Divide(3, 2);
  c_saturated->cd(1);
  h_saturated_NTotHits->Draw("HIST");

  c_saturated->SaveAs(Form("%srun%i_saturated.C", output_directory.c_str(), thisRun.GetRunNumber()));
  c_saturated->SaveAs(Form("%srun%i_saturated.pdf", output_directory.c_str(), thisRun.GetRunNumber()));


    

  // save all files in a histogram list and output to a root file
  TFile *f_out = new TFile(Form("%srun%i_histos.root", output_directory.c_str(), thisRun.GetRunNumber()), "RECREATE");
  f_out->cd();
  hist_list->Write();
  f_out->Close();

  if (run_root_app) app->Run();
  else LogInfo << "Info: Enable SavePlots and PlotsDisplay in AnalysisSettings.json to see plots at end of execution.\n";

  return 0;
}
