//
// Created by Vedantha Srinivas Kasturi on 07.02.2024.
//


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
#include <TVector3.h>
#include <TPaveText.h>

#include "nlohmann/json.hpp"

// move
#include "TLegend.h"
#include "TPolyLine3D.h"

LoggerInit([]{
    Logger::getUserHeader() << "[" << FILENAME << "]";
});

//****************** Computing Functions and Geometry ******************//

bool getCoordinate(int Plane,int Bar, double coord, double &x, double &y, double &z) {

    double barwidth = 12.2; // cm, including the bargap

    // My Geometry Setup

    double topEven10[3] = {-6.2, 133.4, -3.5};
    double topOdd9[3] = { 6.0, 133.4, -7.0 };

    double bottomEven10[3] = {3.1, -131.5, -3.5};
    double bottomOdd9[3] = { -9.1, -131.5, -7.0 };

    double upEven10[3] = {-1.8, -2.0, -101.2};
    double upOdd9[3] = { 1.7, 10.2, -101.2 };

    double downEven10[3] = {1.8, -1.0, 99.6};
    double downOdd9[3] = { -1.7, 11.2, 99.6 };

    // 0: North , 1: South, 2: Bottom , 3: Top , 4: Upstream , 5: Downstream

    if (Plane == kNorthModule || Plane == kSouthModule) {
        std::cout << "N and S are not installed yet!!!! " << std::endl;
        x = y = z = -9999;
    } else if (Plane == kBottomModule) { // Bottom
        // Shifted along z opposing beam entry
        // Y is the same for all bars
        if(Bar == 0 || Bar == 19) {
            std::cout << "Ghost Bar !!!! " << std::endl;
            x = y = z = -9999;
        } else if (Bar % 2 == 0) { // Even
            if(Bar >= 10) {
                x = bottomEven10[0] + (Bar-10) * barwidth;
                y = bottomEven10[1];
                z = bottomEven10[2] + coord;
            } else {
                x = bottomEven10[0] - (10- Bar) * barwidth;
                y = bottomEven10[1];
                z = bottomEven10[2] + coord;
            }
        } else { // Odd
            if(Bar >= 9) {
                x = bottomOdd9[0] + (Bar-9) * barwidth;
                y = bottomOdd9[1];
                z = bottomOdd9[2] + coord;
            } else {
                x = bottomOdd9[0] - (9- Bar) * barwidth;
                y = bottomOdd9[1];
                z = bottomOdd9[2] + coord;
            }
        }
    } else if (Plane == kTopModule){ // Top
        // Shifted along z opposing beam entry and along x in negative direction
        // Y is the same for all bars
        if (Bar % 2 == 0) { // Even
            if(Bar >= 10) {
                x = topEven10[0] - (Bar-10) * barwidth;
                y = topEven10[1];
                z = topEven10[2] + coord;
            } else {
                x = topEven10[0] + (10- Bar) * barwidth;
                y = topEven10[1];
                z = topEven10[2] + coord;
            }
        } else { // Odd
            if(Bar >= 9) {
                x = topOdd9[0] - (Bar-9) * barwidth;
                y = topOdd9[1];
                z = topOdd9[2] + coord;
            } else {
                x = topOdd9[0] + (9- Bar) * barwidth;
                y = topOdd9[1];
                z = topOdd9[2] + coord;
            }
        }
    } else if (Plane == kUpstreamModule){ // Upstream
        // Standard
        // Z is the same for all bars
        if (Bar % 2 == 0) { // Even
            if (Bar >= 10) {
                x = upEven10[0] + coord;
                y = upEven10[1] - (Bar-10) * barwidth;
                z = upEven10[2];
            } else {
                x = upEven10[0] + coord;
                y = upEven10[1] + (10 - Bar) * barwidth;
                z = upEven10[2];
            }
        } else { // Odd
            if (Bar >= 9) {
                x = upOdd9[0] + coord;
                y = upOdd9[1] - (Bar-9) * barwidth;
                z = upOdd9[2];
            } else {
                x = upOdd9[0] + coord;
                y = upOdd9[1] + (9 - Bar) * barwidth;
                z = upOdd9[2];
            }
        }
    } else if (Plane == kDownstreamModule){ // Downstream
        // Standard
        // Z is the same for all bars
        if (Bar % 2 == 0) { // Even
            if (Bar >= 10) {
                x = downEven10[0] + coord;
                y = downEven10[1] - (Bar-10) * barwidth;
                z = downEven10[2];
            } else {
                x = downEven10[0] + coord;
                y = downEven10[1] + (10 - Bar) * barwidth;
                z = downEven10[2];
            }
        } else { // Odd
            if (Bar >= 9) {
                x = downOdd9[0] + coord;
                y = downOdd9[1] - (Bar-9) * barwidth;
                z = downOdd9[2];
            } else {
                x = downOdd9[0] + coord;
                y = downOdd9[1] + (9 - Bar) * barwidth;
                z = downOdd9[2];
            }
        }
    } else {
        std::cout << "Ghost Plane !!!! " << std::endl;
        x = y = z = -9999;
    }

    return true;
}

bool getCoordinate1(int Plane,int Bar, double coord, double &x, double &y, double &z) {

    double barwidth = 12.2; // cm, including the bargap
    double stagger = 3.5/2; // cm, stagger between bars

    //Standard Geometry Setup

    double bottom[3] = {-2.8, -131.5, -5.3};
    double top[3] = {2, 133.4, -5.3};
    double downstream[3] = {0, 5.5, 99.6};
    double upstream[3] = {0, 5.5, 101.2};

    // 0: North , 1: South, 2: Bottom , 3: Top , 4: Upstream , 5: Downstream

    if (Plane == kNorthModule || Plane == kSouthModule) {
        std::cout << "N and S are not installed yet!!!! " << std::endl;
        x = y = z = -9999;
    } else if (Plane == kBottomModule) { // Bottom

        if(Bar%2 ==0 ){
            stagger = -stagger;
        } else {
            stagger = stagger;
        }
        x = bottom[0] - (10-Bar-0.5) * barwidth;
        y = bottom[1];
        z = bottom[2] + coord + stagger;
    } else if (Plane == kTopModule){ // Top
        if(Bar%2 ==0 ){
            stagger = -stagger;
        } else {
            stagger = stagger;
        }
        x = top[0] + (10-Bar-0.5) * barwidth;
        y = top[1];
        z = top[2] + coord + stagger;
    } else if (Plane == kUpstreamModule){ // Upstream
        if(Bar%2 ==0 ){
            stagger = -stagger;
        } else {
            stagger = stagger;
        }
        x = upstream[0] + coord + stagger;
        y = upstream[1] + (10-Bar-0.5) * barwidth;
        z = upstream[2];
    } else if (Plane == kDownstreamModule){ // Downstream
        if(Bar%2 ==0 ){
            stagger = -stagger;
        } else {
            stagger = stagger;
        }
        x = downstream[0] + coord + stagger;
        y = downstream[1] - (10-Bar-0.5) * barwidth;
        z = downstream[2];
    } else {
        std::cout << "Ghost Plane !!!! " << std::endl;
        x = y = z = -9999;
    }

    return true;
}

bool getBarEnds(int Plane, int Bar, double &x1, double &y1, double &z1, double &x2, double &y2, double &z2) { // x1, y1, z1 are the coordinates of the left end of the bar, x2, y2, z2 are the coordinates of the right end of the bar

    double barwidth = 12.2; // cm, including the bargap

    // My Geometry Setup

    double topEven10[3] = {-6.2, 133.4, -3.5};
    double topOdd9[3] = { 6.0, 133.4, -7.0 };

    double bottomEven10[3] = {3.1, -131.5, -3.5};
    double bottomOdd9[3] = { -9.1, -131.5, -7.0 };

    double upEven10[3] = {-1.8, -2.0, -101.2};
    double upOdd9[3] = { 1.7, 10.2, -101.2 };

    double downEven10[3] = {1.8, -1.0, 99.6};
    double downOdd9[3] = { -1.7, 11.2, 99.6 };

    // 0: North , 1: South, 2: Bottom , 3: Top , 4: Upstream , 5: Downstream

    if (Plane == 0 || Plane == 1) {
        std::cout << "N and S are not installed yet!!!! " << std::endl;
        x1 = y1 = z1 = x2 = y2 = z2 = 0;
    } else if (Plane == 2) { // Bottom
        // Shifted along z opposing beam entry
        // Y is the same for all bars
        if(Bar == 0 || Bar == 19) {
            std::cout << "Ghost Bar !!!! " << std::endl;
            x1 = y1 = z1 = x2 = y2 = z2 = 0;
        } else if (Bar % 2 == 0) { // Even
            if(Bar >= 10) {
                x1 = bottomEven10[0] + (Bar-10) * barwidth;
                y1 = bottomEven10[1];
                z1 = bottomEven10[2] - 110;
                x2 = bottomEven10[0] + (Bar-10) * barwidth;
                y2 = bottomEven10[1];
                z2 = bottomEven10[2] + 110;
            } else {
                x1 = bottomEven10[0] - (10- Bar) * barwidth;
                y1 = bottomEven10[1];
                z1 = bottomEven10[2] - 110;
                x2 = bottomEven10[0] - (10- Bar) * barwidth;
                y2 = bottomEven10[1];
                z2 = bottomEven10[2] + 110;
            }
        } else { // Odd
            if(Bar >= 9) {
                x1 = bottomOdd9[0] + (Bar-9) * barwidth;
                y1 = bottomOdd9[1];
                z1 = bottomOdd9[2] - 110;
                x2 = bottomOdd9[0] + (Bar-9) * barwidth;
                y2 = bottomOdd9[1];
                z2 = bottomOdd9[2] + 110;
            } else {
                x1 = bottomOdd9[0] - (9- Bar) * barwidth;
                y1 = bottomOdd9[1];
                z1 = bottomOdd9[2] - 110;
                x2 = bottomOdd9[0] - (9- Bar) * barwidth;
                y2 = bottomOdd9[1];
                z2 = bottomOdd9[2] + 110;
            }
        }
    } else if (Plane == 3){ // Top
        // Shifted along z opposing beam entry and along x in negative direction
        // Y is the same for all bars
        if (Bar % 2 == 0) { // Even
            if(Bar >= 10) {
                x1 = topEven10[0] - (Bar-10) * barwidth;
                y1 = topEven10[1];
                z1 = topEven10[2] - 110;
                x2 = topEven10[0] - (Bar-10) * barwidth;
                y2 = topEven10[1];
                z2 = topEven10[2] + 110;
            } else {
                x1 = topEven10[0] + (10- Bar) * barwidth;
                y1 = topEven10[1];
                z1 = topEven10[2] - 110;
                x2 = topEven10[0] + (10- Bar) * barwidth;
                y2 = topEven10[1];
                z2 = topEven10[2] + 110;
            }
        } else { // Odd
            if(Bar >= 9) {
                x1 = topOdd9[0] - (Bar-9) * barwidth;
                y1 = topOdd9[1];
                z1 = topOdd9[2] - 110;
                x2 = topOdd9[0] - (Bar-9) * barwidth;
                y2 = topOdd9[1];
                z2 = topOdd9[2] + 110;
            } else {
                x1 = topOdd9[0] + (9- Bar) * barwidth;
                y1 = topOdd9[1];
                z1 = topOdd9[2] - 110;
                x2 = topOdd9[0] + (9- Bar) * barwidth;
                y2 = topOdd9[1];
                z2 = topOdd9[2] + 110;
            }
        }
    } else if (Plane == 4){ // Upstream
        // Standard
        // Z is the same for all bars
        if (Bar % 2 == 0) { // Even
            if (Bar >= 10) {
                x1 = upEven10[0] - 110;
                y1 = upEven10[1] - (Bar-10) * barwidth;
                z1 = upEven10[2];
                x2 = upEven10[0] + 110;
                y2 = upEven10[1] - (Bar-10) * barwidth;
                z2 = upEven10[2];
            } else {
                x1 = upEven10[0] - 110;
                y1 = upEven10[1] + (10 - Bar) * barwidth;
                z1 = upEven10[2];
                x2 = upEven10[0] + 110;
                y2 = upEven10[1] + (10 - Bar) * barwidth;
                z2 = upEven10[2];


            }
        } else { // Odd
            if (Bar >= 9) {
                x1 = upOdd9[0] - 110;
                y1 = upOdd9[1] - (Bar-9) * barwidth;
                z1 = upOdd9[2];
                x2 = upOdd9[0] + 110;
                y2 = upOdd9[1] - (Bar-9) * barwidth;
                z2 = upOdd9[2] + 100;
            } else {
                x1 = upOdd9[0] - 110;
                y1 = upOdd9[1] + (9 - Bar) * barwidth;
                z1 = upOdd9[2];
                x2 = upOdd9[0] + 110;
                y2 = upOdd9[1] + (9 - Bar) * barwidth;
                z2 = upOdd9[2] + 100;
            }
        }
    } else if (Plane == 5){ // Downstream
        // Standard
        // Z is the same for all bars
        if (Bar % 2 == 0) { // Even
            if (Bar >= 10) {
                x1 = downEven10[0] - 110;
                y1 = downEven10[1] - (Bar-10) * barwidth;
                z1 = downEven10[2];
                x2 = downEven10[0] + 110;
                y2 = downEven10[1] - (Bar-10) * barwidth;
                z2 = downEven10[2];
            } else {
                x1 = downEven10[0] - 110;
                y1 = downEven10[1] + (10 - Bar) * barwidth;
                z1 = downEven10[2];
                x2 = downEven10[0] + 110;
                y2 = downEven10[1] + (10 - Bar) * barwidth;
                z2 = downEven10[2];
            }
        } else { // Odd
            if (Bar >= 9) {
                x1 = downOdd9[0] - 110;
                y1 = downOdd9[1] - (Bar-9) * barwidth;
                z1 = downOdd9[2];
                x2 = downOdd9[0] + 110;
                y2 = downOdd9[1] - (Bar-9) * barwidth;
                z2 = downOdd9[2];
            } else {
                x1 = downOdd9[0] - 110;
                y1 = downOdd9[1] + (9 - Bar) * barwidth;
                z1 = downOdd9[2];
                x2 = downOdd9[0] + 110;
                y2 = downOdd9[1] + (9 - Bar) * barwidth;
                z2 = downOdd9[2];
            }
        }
    } else {
        std::cout << "Ghost Plane !!!! " << std::endl;
        x1 = y1 = z1 = x2 = y2 = z2 = 0;
    }

    return true;
}

void getAngles(double x1, double y1, double z1, double x2, double y2, double z2, double &angleZ, double &angleY, double &phi) {

    TVector3 v1(x1, y1, z1);
    TVector3 v2(x2, y2, z2);

    TVector3 v = v2 - v1;

    angleZ = v.Angle(TVector3(0, 0, 1));
    angleY = v.Angle(TVector3(0, 1, 0));

    phi = v.Phi();

}

struct point{
    double x;
    double y;
    double z;
    int plane;
    double charge;
    double position;
    double time;
    int bar;
};

struct track {
    point start;
    point end;
    double distance;
    double timeDiff;
    double velocity;
    double angleZ;
    double angleY;
    double phi;
    int orientation; // 0: horizontal, 1: vertical , 2: diagonal
};

void drawTrackInCube(const track& tr) {
    // Create a TCanvas to draw on
    TCanvas canvas("canvas", "3D Track in Cube", 800, 600);

    // Create a TPolyLine3D object to represent the track
    TPolyLine3D trackLine(2);

    // Set the points of the track line
    trackLine.SetPoint(0, tr.start.x, tr.start.y, tr.start.z);
    trackLine.SetPoint(1, tr.end.x, tr.end.y, tr.end.z);

    // Draw the track line
    trackLine.Draw();

    // Define the corners of the cube
    double cubeVertices[8][3] = {
            {0, 0, 0},
            {1, 0, 0},
            {1, 1, 0},
            {0, 1, 0},
            {0, 0, 1},
            {1, 0, 1},
            {1, 1, 1},
            {0, 1, 1}
    };

    // Define the edges of the cube
    int cubeEdges[12][2] = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},
            {4, 5}, {5, 6}, {6, 7}, {7, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    // Draw the cube
    for (int i = 0; i < 12; ++i) {
        double x1 = cubeVertices[cubeEdges[i][0]][0];
        double y1 = cubeVertices[cubeEdges[i][0]][1];
        double z1 = cubeVertices[cubeEdges[i][0]][2];
        double x2 = cubeVertices[cubeEdges[i][1]][0];
        double y2 = cubeVertices[cubeEdges[i][1]][1];
        double z2 = cubeVertices[cubeEdges[i][1]][2];
        TPolyLine3D cubeEdge(2);
        cubeEdge.SetPoint(0, x1, y1, z1);
        cubeEdge.SetPoint(1, x2, y2, z2);
        cubeEdge.Draw();
    }

    // Update the canvas
    canvas.Update();
}

//*********************************************************************//

int main(int argc, char *argv[]) {

    //***************************** Some standard stuff  ***************************//

    CmdLineParser clp;

    clp.getDescription() << "> analysisPrime reads the data reonstructed by the tofReco and performs an in depth analysis and produces the plots" << std::endl;
//    clp.getDescription() << "> and performs a primary event reconstruction, mainly for monitoring." << std::endl;

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

    LogThrowIf(clp.isNoOptionTriggered(), "No option was provided.");

    LogInfo << "Provided arguments: " << std::endl;
    LogInfo << clp.getValueSummary() << std::endl << std::endl;

    std::string software = std::string(
            clp.getOptionVal<std::string>("whichSoftware", "linux")); // if no arg, deafult is linux
    if (software == "linux") LogInfo << "Linux software" << std::endl;
    else if (software == "windows") LogInfo << "Windows software" << std::endl;
    else {
        LogError << "Parsed software not recognized, has to be 'windows' or 'linux'" << std::endl;
        return 1;
    }

    // read settings from json file
    std::string ana_settings_file = clp.getOptionVal<std::string>("appSettings");
    std::ifstream ana_settings_stream(ana_settings_file);
    LogInfo << "Reading analysis settings from " << ana_settings_file.c_str() << std::endl;
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

    thisRun.RunSetVerbose(clp.isOptionTriggered("verboseMode")); // TODO check this
    thisRun.RunSetSoftwareType(software);
    thisRun.RunSetInputFilePath(run_full_path);
    thisRun.RunReadFilename();
    thisRun.RunSaveSettings();
    thisRun.RunQualityCheck();
    thisRun.RunSetAnalysisOptions(ana_settings_file);
    thisRun.RunGetInfo(); // comment to reduce output
    thisRun.RunLoadHits();
    thisRun.RunPrintErrors();

    //**************************************************************************************************//

    //////////////////////////////////////////////////////////////
    // ROOT app and objects to read the data in the Run
    auto *app = new TApplication("myapp", &argc, argv);

    //*****************************************************************************************************//

    // Add the histograms to the app

    TH1F *hTrackDist = new TH1F("hTrackDist", "Track distance", 100, -5, 600);
    TH1F *hTrackToF = new TH1F("hTrackToF", "Track time of flight", 100, -5, 70);
    TH1F *hTrackVelocity = new TH1F("hTrackVelocity", "Track velocity", 100, 0, 100);
    //signal bar
    TH1D *hSignalBar = new TH1D("hSignalBar", "Signal Bar", 21, 0, 21);
    // Signal plane
    TH1D *hSignalPlane = new TH1D("hSignalPlane", "Signal Plane", 20, 0, 7);
    // Signal position
    TH1D *hSignalPosition = new TH1D("hSignalPosition", "Signal Position", 100, -110, 110);
    // Signal time
    TH1D *hSignalTime = new TH1D("hSignalTime", "Signal Time", 100, 0, 100);
    // track Charge
    TH1D *hTrackCharge = new TH1D("hTrackCharge", "Track Charge", 100, 0, 200);
    // track angleZ
    TH1D *hTrackAngleZ = new TH1D("hTrackAngleZ", "Track AngleZ", 100, 0, 3.2);
    // track angleY
    TH1D *hTrackAngleY = new TH1D("hTrackAngleY", "Track AngleY", 100, 0, 3.2);
    // track phi
    TH1D *hTrackPhi = new TH1D("hTrackPhi", "Track Phi", 100, -3.2, 3.2);
    // track orientation
    TH1D *hTrackOrientation = new TH1D("hTrackOrientation", "Track Orientation", 10, 0, 5);
    // time bias
    TH1D *hTimeBias = new TH1D("hTimeBias", "Time Bias", 100, -10, 10);

    // time bias in upstream
    TH1D *hTimeBiasUpstream = new TH1D("hTimeBiasUpstream", "Time Bias Upstream", 100, -10, 10);
    // time bias in downstream
    TH1D *hTimeBiasDownstream = new TH1D("hTimeBiasDownstream", "Time Bias Downstream", 100, -10, 10);
    // time bias in top
    TH1D *hTimeBiasTop = new TH1D("hTimeBiasTop", "Time Bias Top", 100, -10, 10);
    // time bias in bottom
    TH1D *hTimeBiasBottom = new TH1D("hTimeBiasBottom", "Time Bias Bottom", 100, -10, 10);



    // Track distance vs velocity
    TH2D *hTrackDistVsVelocity = new TH2D("hTrackDistVsVelocity", "Track Distance vs Velocity", 100, 0, 100, 100, 0, 600);
    // Track orientation vs velocity
    TH2D *hTrackOrientationVsVelocity = new TH2D("hTrackOrientationVsVelocity", "Track Orientation vs Velocity", 100, 0, 100, 10, 0, 4);
    // Charge vs distance
    TH2D *hTrackChargeVsDistance = new TH2D("hTrackChargeVsDistance", "Track Charge vs Distance", 100, 0, 200, 100, 0, 600);
    // Charge vs velocity
    TH2D *hTrackChargeVsVelocity = new TH2D("hTrackChargeVsVelocity", "Track Charge vs Velocity", 100, 0, 200, 100, 0, 100);
    // velocity vs signal Plane
    TH2D *hTrackVelocityVsPlane = new TH2D("hTrackVelocityVsPlane", "Track Velocity vs Plane", 100, 0, 100, 10, 0, 10);



    //*****************************************************************************************************//
    track thisTrack{};

//    std::vector<track> tracks;
    // Store the tracks in a root file

    std::string outdi = analysis_settings_file["outputDirectory"];
    // add the run number to the track file
    std::string runNumber = std::to_string(clp.getOptionVal<int>("runNumber"));

    TFile *trackFile = new TFile((outdi + "/tracks_"+runNumber+".root").c_str(), "RECREATE");
    TTree *tree = new TTree("tracks", "tracks");
    tree->Branch("start_x", &thisTrack.start.x, "start_x/D");
    tree->Branch("start_y", &thisTrack.start.y, "start_y/D");
    tree->Branch("start_z", &thisTrack.start.z, "start_z/D");
    tree->Branch("start_plane", &thisTrack.start.plane, "start_plane/I");
    tree->Branch("start_charge", &thisTrack.start.charge, "start_charge/D");
    tree->Branch("end_x", &thisTrack.end.x, "end_x/D");
    tree->Branch("end_y", &thisTrack.end.y, "end_y/D");
    tree->Branch("end_z", &thisTrack.end.z, "end_z/D");
    tree->Branch("end_plane", &thisTrack.end.plane, "end_plane/I");
    tree->Branch("end_charge", &thisTrack.end.charge, "end_charge/D");
    tree->Branch("distance", &thisTrack.distance, "distance/D");
    tree->Branch("timeDiff", &thisTrack.timeDiff, "timeDiff/D");
    tree->Branch("velocity", &thisTrack.velocity, "velocity/D");
    tree->Branch("angleZ", &thisTrack.angleZ, "angleZ/D");
    tree->Branch("angleY", &thisTrack.angleY, "angleY/D");
    tree->Branch("phi", &thisTrack.phi, "phi/D");
    tree->Branch("orientation", &thisTrack.orientation, "orientation/I");
    tree->Branch("start_time", &thisTrack.start.time, "start_time/D");
    tree->Branch("end_time", &thisTrack.end.time, "end_time/D");
    tree->Branch("start_position", &thisTrack.start.position, "start_position/D");
    tree->Branch("end_position", &thisTrack.end.position, "end_position/D");
    tree->Branch("start_bar", &thisTrack.start.bar, "start_bar/I");
    tree->Branch("end_bar", &thisTrack.end.bar, "end_bar/I");

    //*************************************** Actual Code ***************************************************//

//    double timeshift[6] = {0,0,-0.05, 0.17, 0.03, -0.79};
//    double timeshift[6] = {0,0,0.33, 0.0, -0.2, +0.76};
    double timeshift[6] = {0,0,0., 0., 0., 0.};

    LogInfo << "Number of events: " << thisRun.GetRunEventsList().size() << std::endl;
    int event_counter = 0;
    for (auto  eventit : thisRun.GetRunEventsList()) { // Loop over the events
        // LogInfo << "enter event loop" << std::endl;

        auto signals = eventit.GetEventSignalsList();

        // Iterate over the signals in the event

        for (auto signalit : signals) {
            // LogInfo << "enter signal loop" << std::endl;

            hSignalBar->Fill(signalit.GetSignalBar());
            hSignalPlane->Fill(signalit.GetSignalPlane());
            hSignalPosition->Fill(signalit.GetSignalPosition());
            hSignalTime->Fill(signalit.GetSignalTime());

        }


        for (auto it1 = signals.begin(); it1 != signals.end(); ++it1) { // Loop over the signals
            for (auto it2 = std::next(it1); it2 != signals.end(); ++it2) {

                // LogInfo << "enter signal loop" << std::endl;
                auto signal1 = *it1;
                auto signal2 = *it2;

                if (signal1.GetSignalType() == 3 && signal2.GetSignalType() == 3) {

                    if(signal1.GetSignalBar() == signal2.GetSignalBar() || signal1.GetSignalPlane() == signal2.GetSignalPlane()) continue; // Same bar or same plane
                    if(signal1.GetSignalTime() == -1 || signal2.GetSignalTime() == -1) continue; // No time information


                    double x1, y1, z1, x2, y2, z2;
                    getCoordinate(signal1.GetSignalPlane(), signal1.GetSignalBar(), signal1.GetSignalPosition(), x1, y1,
                                  z1);
                    getCoordinate(signal2.GetSignalPlane(), signal2.GetSignalBar(), signal2.GetSignalPosition(), x2, y2,
                                  z2);

                    double distance = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
                    double timeDiff = abs(signal2.GetSignalTime() - signal1.GetSignalTime());
                    double velocity = distance / timeDiff;

                    double angleZ, angleY, phi;


                    // Check order of the signals

                    if(signal1.GetSignalTime() > signal2.GetSignalTime()){
                        getAngles(x2, y2, z2, x1, y1, z1, angleZ, angleY, phi);

                        thisTrack.start.x = x2;
                        thisTrack.start.y = y2;
                        thisTrack.start.z = z2;
                        thisTrack.end.x = x1;
                        thisTrack.end.y = y1;
                        thisTrack.end.z = z1;
                        thisTrack.start.plane = signal2.GetSignalPlane();
                        thisTrack.end.plane = signal1.GetSignalPlane();
                        thisTrack.start.charge = (signal2.GetSignalHitMin().GetHitVoltageIntegral()+ signal2.GetSignalHitMax().GetHitVoltageIntegral());
                        thisTrack.end.charge = (signal1.GetSignalHitMin().GetHitVoltageIntegral()+ signal1.GetSignalHitMax().GetHitVoltageIntegral());
                        thisTrack.start.position = signal2.GetSignalPosition();
                        thisTrack.end.position = signal1.GetSignalPosition();
                        thisTrack.start.time = signal2.GetSignalTime();
                        thisTrack.end.time = signal1.GetSignalTime();
                        thisTrack.start.bar = signal2.GetSignalBar();
                        thisTrack.end.bar = signal1.GetSignalBar();


                        thisTrack.distance = distance;
                        thisTrack.timeDiff = timeDiff + timeshift[signal2.GetSignalPlane()] - timeshift[signal1.GetSignalPlane()];
                        thisTrack.velocity = velocity;
                        thisTrack.angleZ = angleZ;
                        thisTrack.angleY = angleY;
                        thisTrack.phi = phi;


                    } else {
                        getAngles(x1, y1, z1, x2, y2, z2, angleZ, angleY, phi);
                        thisTrack.start.x = x1;
                        thisTrack.start.y = y1;
                        thisTrack.start.z = z1;
                        thisTrack.end.x = x2;
                        thisTrack.end.y = y2;
                        thisTrack.end.z = z2;
                        thisTrack.start.plane = signal1.GetSignalPlane();
                        thisTrack.end.plane = signal2.GetSignalPlane();
                        thisTrack.start.charge = (signal1.GetSignalHitMin().GetHitVoltageIntegral()+ signal1.GetSignalHitMax().GetHitVoltageIntegral());
                        thisTrack.end.charge = (signal2.GetSignalHitMin().GetHitVoltageIntegral()+ signal2.GetSignalHitMax().GetHitVoltageIntegral());
                        thisTrack.start.position = signal1.GetSignalPosition();
                        thisTrack.end.position = signal2.GetSignalPosition();
                        thisTrack.start.time = signal1.GetSignalTime();
                        thisTrack.end.time = signal2.GetSignalTime();
                        thisTrack.start.bar = signal1.GetSignalBar();
                        thisTrack.end.bar = signal2.GetSignalBar();


                        thisTrack.distance = distance;
                        thisTrack.timeDiff = timeDiff + timeshift[signal2.GetSignalPlane()] - timeshift[signal1.GetSignalPlane()];
                        thisTrack.velocity = velocity;
                        thisTrack.angleZ = angleZ;
                        thisTrack.angleY = angleY;
                        thisTrack.phi = phi;
                    }
                    //getAngles(x1, y1, z1, x2, y2, z2, angleZ, angleY, phi);

                    // get signal 1 left and right bar ends
//                    double xL1,yL1,zL1,xR1,yR1,zR1;
//                    getBarEnds(signal1.GetSignalPlane(),signal1.GetSignalBar(),xL1,yL1,zL1,xR1,yR1,zR1);
//
//                    // get signal 2 left and right ends
//                    double xL2,yL2,zL2,xR2,yR2,zR2;
//                    getBarEnds(signal2.GetSignalPlane(),signal2.GetSignalBar(),xL2,yL2,zL2,xR2,yR2,zR2);
//
//                    // calculate the distance between x1 and xL1 and xR1
//                    double distance1 = sqrt(pow(x1 - xL1, 2) + pow(y1 - yL1, 2) + pow(z1 - zL1, 2));
//                    double distance2 = sqrt(pow(x1 - xR1, 2) + pow(y1 - yR1, 2) + pow(z1 - zR1, 2));
//
//                    // calculate the distance between x2 and xL2 and xR2
//                    double distance3 = sqrt(pow(x2 - xL2, 2) + pow(y2 - yL2, 2) + pow(z2 - zL2, 2));
//                    double distance4 = sqrt(pow(x2 - xR2, 2) + pow(y2 - yR2, 2) + pow(z2 - zR2, 2));







                    if (signal1.GetSignalPlane() == 3){ // Top First
                        if(signal2.GetSignalPlane() == 2){ // Bottom Second
                            thisTrack.orientation = 1;
                        } else {
                            thisTrack.orientation = 2;
                        }
                    } else if (signal1.GetSignalPlane() == 2){  // Bottom First
                        if(signal2.GetSignalPlane() == 3){ // Top Second
                            thisTrack.orientation = 1;
                        } else {
                            thisTrack.orientation = 2;
                        }
                    } else if (signal1.GetSignalPlane() == 4){ // upstream First
                        if(signal2.GetSignalPlane() == 5){
                            thisTrack.orientation = 0;
                        } else {
                            thisTrack.orientation = 2;
                        }
                    } else if (signal1.GetSignalPlane() == 5){ // downstream First
                        if(signal2.GetSignalPlane() == 4){
                            thisTrack.orientation = 0;
                        } else {
                            thisTrack.orientation = 2;
                        }
                    } else {
                        thisTrack.orientation = 1;
                    }

                    double timebias = thisTrack.timeDiff - thisTrack.distance / 30;

                    if(signal1.GetSignalPlane() == 3){
                        hTimeBiasTop->Fill(timebias);
                    } else if (signal1.GetSignalPlane() == 2){
                        hTimeBiasBottom->Fill(timebias);
                    } else if (signal1.GetSignalPlane() == 4){
                        hTimeBiasUpstream->Fill(timebias);
                    } else if (signal1.GetSignalPlane() == 5){
                        hTimeBiasDownstream->Fill(timebias);
                    }

                    // Only see tracks with velocity greater than 30
//                    if(thisTrack.velocity < 30) continue;


                    hTrackDist->Fill(thisTrack.distance);
                    hTrackToF->Fill(thisTrack.timeDiff);
                    hTrackVelocity->Fill(thisTrack.velocity);
                    hTrackCharge->Fill(thisTrack.start.charge + thisTrack.end.charge);
                    hTrackAngleZ->Fill(thisTrack.angleZ);
                    hTrackAngleY->Fill(thisTrack.angleY);
                    hTrackPhi->Fill(thisTrack.phi);
                    hTrackOrientation->Fill(thisTrack.orientation);
                    hTimeBias->Fill(timebias);

                    hTrackDistVsVelocity->Fill(thisTrack.velocity, thisTrack.distance);
                    hTrackOrientationVsVelocity->Fill(thisTrack.velocity, thisTrack.orientation);
                    hTrackChargeVsDistance->Fill(thisTrack.start.charge + thisTrack.end.charge, thisTrack.distance);
                    hTrackChargeVsVelocity->Fill(thisTrack.start.charge + thisTrack.end.charge, thisTrack.velocity);
                    hTrackVelocityVsPlane->Fill(thisTrack.velocity, thisTrack.start.plane);

                    tree->Fill();
//                    tracks.push_back(thisTrack);

                }
            }
        }



    }

    trackFile->Write();
    trackFile->Close();



    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
    c1->Divide(2, 2);
    c1->cd(1);
    hSignalBar->Draw();
    hSignalBar->GetXaxis()->SetTitle("Bar");
    hSignalBar->GetYaxis()->SetTitle("Counts");
    c1->cd(2);
    hSignalPlane->Draw();
    hSignalPlane->GetXaxis()->SetTitle("Plane");
    hSignalPlane->GetYaxis()->SetTitle("Counts");
    c1->cd(3);
    hSignalPosition->Draw();
    hSignalPosition->GetXaxis()->SetTitle("Position [cm]");
    hSignalPosition->GetYaxis()->SetTitle("Counts");
    c1->cd(4);
    hSignalTime->Draw();
    hSignalTime->GetXaxis()->SetTitle("Time [ns]");
    hSignalTime->GetYaxis()->SetTitle("Counts");
    c1->Draw();

    TCanvas *c2 = new TCanvas("c2", "c2", 800, 600);

    c2->Divide(2, 2);
    c2->cd(1);
    hTrackDist->Draw();
    hTrackDist->GetXaxis()->SetTitle("Distance [cm]");
    hTrackDist->GetYaxis()->SetTitle("Counts");
    c2->cd(2);
    hTrackToF->Draw();
    hTrackToF->GetXaxis()->SetTitle("Time difference [ns]");
    hTrackToF->GetYaxis()->SetTitle("Counts");
    c2->cd(3);
    hTrackVelocity->Draw();
    hTrackVelocity->GetXaxis()->SetTitle("Velocity [cm/ns]");
    hTrackVelocity->GetYaxis()->SetTitle("Counts");
    // fit a gaussian
    TF1 *f = new TF1("f", "gaus", 0, 100);
    hTrackVelocity->Fit("f", "R");
    // show the fit
    hTrackVelocity->GetFunction("f")->SetLineColor(kRed);
    hTrackVelocity->GetFunction("f")->SetLineWidth(1);
    hTrackVelocity->GetFunction("f")->Draw("same");
    TLegend *leg1 = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg1->AddEntry(hTrackVelocity, "Velocity", "l");
    leg1->AddEntry(hTrackVelocity->GetFunction("f"), "Gaussian fit", "l");
    leg1->Draw();
    // Show the fit values on the canvas
    TPaveText *pt = new TPaveText(0.7, 0.5, 0.9, 0.7, "NDC");
    pt->AddText(Form("Mean = %.2f", hTrackVelocity->GetFunction("f")->GetParameter(1)));
    pt->AddText(Form("Sigma = %.2f", hTrackVelocity->GetFunction("f")->GetParameter(2)));
    pt->Draw();



    c2->cd(4);
    hTrackOrientation->Draw();
    hTrackOrientation->GetXaxis()->SetTitle("Orientation");
    hTrackOrientation->GetYaxis()->SetTitle("Counts");
    c2->Draw();

    TCanvas *c3 = new TCanvas("c3", "c3", 800, 600);
    c3->Divide(2, 2);
    c3->cd(1);
    hTrackCharge->Draw();
    hTrackCharge->GetXaxis()->SetTitle("Charge [mV*ns]");
    hTrackCharge->GetYaxis()->SetTitle("Counts");
    // fit a landau
    TF1 *f1 = new TF1("f1", "landau", 0, 100);
    hTrackCharge->Fit("f1", "R");
    // show the fit
    hTrackCharge->GetFunction("f1")->SetLineColor(kRed);
    hTrackCharge->GetFunction("f1")->SetLineWidth(1);
    hTrackCharge->GetFunction("f1")->Draw("same");
    TLegend *leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg->AddEntry(hTrackCharge, "Charge", "l");
    leg->AddEntry(hTrackCharge->GetFunction("f1"), "Landau fit", "l");
    leg->Draw();
    // Show the fit values on the canvas
    TPaveText *pt1 = new TPaveText(0.7, 0.5, 0.9, 0.7, "NDC");
    pt1->AddText(Form("MPV = %.2f", hTrackCharge->GetFunction("f1")->GetParameter(1)));
    pt1->AddText(Form("Sigma = %.2f", hTrackCharge->GetFunction("f1")->GetParameter(2)));
    pt1->Draw();



    c3->cd(2);
    hTrackAngleZ->Draw();
    hTrackAngleZ->GetXaxis()->SetTitle("Angle [rad]");
    hTrackAngleZ->GetYaxis()->SetTitle("Counts");
    c3->cd(3);
    hTrackAngleY->Draw();
    hTrackAngleY->GetXaxis()->SetTitle("Angle [rad]");
    hTrackAngleY->GetYaxis()->SetTitle("Counts");
    c3->cd(4);
    hTrackPhi->Draw();
    hTrackPhi->GetXaxis()->SetTitle("Phi [rad]");
    hTrackPhi->GetYaxis()->SetTitle("Counts");
    c3->Draw();

    TCanvas *c4 = new TCanvas("c4", "c4", 800, 600);
    hTrackDistVsVelocity->Draw("colz");
    hTrackDistVsVelocity->GetXaxis()->SetTitle("Velocity [cm/ns]");
    hTrackDistVsVelocity->GetYaxis()->SetTitle("Distance [cm]");
    c4->Draw();

    TCanvas *c5 = new TCanvas("c5", "c5", 800, 600);
    c5->Divide(2, 2);
    c5->cd(1);
    hTrackOrientationVsVelocity->Draw("colz");
    hTrackOrientationVsVelocity->GetXaxis()->SetTitle("Velocity [cm/ns]");
    hTrackOrientationVsVelocity->GetYaxis()->SetTitle("Orientation");
    c5->cd(2);
    hTrackChargeVsDistance->Draw("colz");
    hTrackChargeVsDistance->GetYaxis()->SetTitle("Distance [cm]");
    hTrackChargeVsDistance->GetXaxis()->SetTitle("Charge [mV*ns]");
    c5->cd(3);
    hTrackChargeVsVelocity->Draw("colz");
    hTrackChargeVsVelocity->GetYaxis()->SetTitle("Velocity [cm/ns]");
    hTrackChargeVsVelocity->GetXaxis()->SetTitle("Charge [mV*ns]");
    c5->cd(4);
    hTrackVelocityVsPlane->Draw("colz");
    hTrackVelocityVsPlane->GetXaxis()->SetTitle("Velocity [cm/ns]");
    hTrackVelocityVsPlane->GetYaxis()->SetTitle("Plane");
    c5->Draw();

    TCanvas *c6 = new TCanvas("c6", "c6", 800, 600);
    hTimeBias->Draw();
    hTimeBias->GetXaxis()->SetTitle("Time Bias [ns]");
    hTimeBias->GetYaxis()->SetTitle("Counts");

    TCanvas *c7 = new TCanvas("c7", "c7", 800, 600);
    c7->Divide(2, 2);
    c7->cd(1);
    hTimeBiasUpstream->Draw();
    hTimeBiasUpstream->GetXaxis()->SetTitle("Time Bias [ns]");
    hTimeBiasUpstream->GetYaxis()->SetTitle("Counts");
    // fit a gaussian
    TF1 *f2 = new TF1("f2", "gaus", -10, 10);
    hTimeBiasUpstream->Fit("f2", "R");
    // show the fit
    hTimeBiasUpstream->GetFunction("f2")->SetLineColor(kRed);
    hTimeBiasUpstream->GetFunction("f2")->SetLineWidth(1);
    hTimeBiasUpstream->GetFunction("f2")->Draw("same");
    TLegend *leg2 = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg2->AddEntry(hTimeBiasUpstream, "Time Bias Upstream", "l");
    leg2->AddEntry(hTimeBiasUpstream->GetFunction("f2"), "Gaussian fit", "l");
    leg2->Draw();

    // Show the fit values on the canvas
    TPaveText *pt2 = new TPaveText(0.7, 0.5, 0.9, 0.7, "NDC");
    pt2->AddText(Form("Mean = %.2f", hTimeBiasUpstream->GetFunction("f2")->GetParameter(1)));
    pt2->AddText(Form("Sigma = %.2f", hTimeBiasUpstream->GetFunction("f2")->GetParameter(2)));
    pt2->Draw();


    c7->cd(2);
    hTimeBiasDownstream->Draw();
    hTimeBiasDownstream->GetXaxis()->SetTitle("Time Bias [ns]");
    hTimeBiasDownstream->GetYaxis()->SetTitle("Counts");
    // fit a gaussian
    TF1 *f3 = new TF1("f3", "gaus", -10, 10);
    hTimeBiasDownstream->Fit("f3", "R");
    // show the fit
    hTimeBiasDownstream->GetFunction("f3")->SetLineColor(kRed);
    hTimeBiasDownstream->GetFunction("f3")->SetLineWidth(1);
    hTimeBiasDownstream->GetFunction("f3")->Draw("same");
    TLegend *leg3 = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg3->AddEntry(hTimeBiasDownstream, "Time Bias Downstream", "l");
    leg3->AddEntry(hTimeBiasDownstream->GetFunction("f3"), "Gaussian fit", "l");
    leg3->Draw();

    // Show the fit values on the canvas
    TPaveText *pt3 = new TPaveText(0.7, 0.5, 0.9, 0.7, "NDC");
    pt3->AddText(Form("Mean = %.2f", hTimeBiasDownstream->GetFunction("f3")->GetParameter(1)));
    pt3->AddText(Form("Sigma = %.2f", hTimeBiasDownstream->GetFunction("f3")->GetParameter(2)));
    pt3->Draw();


    c7->cd(3);
    hTimeBiasTop->Draw();
    hTimeBiasTop->GetXaxis()->SetTitle("Time Bias [ns]");
    hTimeBiasTop->GetYaxis()->SetTitle("Counts");
    // fit a gaussian
    TF1 *f4 = new TF1("f4", "gaus", -10, 10);
    hTimeBiasTop->Fit("f4", "R");
    // show the fit
    hTimeBiasTop->GetFunction("f4")->SetLineColor(kRed);
    hTimeBiasTop->GetFunction("f4")->SetLineWidth(1);
    hTimeBiasTop->GetFunction("f4")->Draw("same");
    TLegend *leg4 = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg4->AddEntry(hTimeBiasTop, "Time Bias Top", "l");
    leg4->AddEntry(hTimeBiasTop->GetFunction("f4"), "Gaussian fit", "l");
    leg4->Draw();

    // Show the fit values on the canvas
    TPaveText *pt4 = new TPaveText(0.7, 0.5, 0.9, 0.7, "NDC");
    pt4->AddText(Form("Mean = %.2f", hTimeBiasTop->GetFunction("f4")->GetParameter(1)));
    pt4->AddText(Form("Sigma = %.2f", hTimeBiasTop->GetFunction("f4")->GetParameter(2)));
    pt4->Draw();


    c7->cd(4);
    hTimeBiasBottom->Draw();
    hTimeBiasBottom->GetXaxis()->SetTitle("Time Bias [ns]");
    hTimeBiasBottom->GetYaxis()->SetTitle("Counts");
    // fit a gaussian
    TF1 *f5 = new TF1("f5", "gaus", -10, 10);
    hTimeBiasBottom->Fit("f5", "R");
    // show the fit
    hTimeBiasBottom->GetFunction("f5")->SetLineColor(kRed);
    hTimeBiasBottom->GetFunction("f5")->SetLineWidth(1);
    hTimeBiasBottom->GetFunction("f5")->Draw("same");
    TLegend *leg5 = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg5->AddEntry(hTimeBiasBottom, "Time Bias Bottom", "l");
    leg5->AddEntry(hTimeBiasBottom->GetFunction("f5"), "Gaussian fit", "l");
    leg5->Draw();

    // Show the fit values on the canvas
    TPaveText *pt5 = new TPaveText(0.7, 0.5, 0.9, 0.7, "NDC");
    pt5->AddText(Form("Mean = %.2f", hTimeBiasBottom->GetFunction("f5")->GetParameter(1)));
    pt5->AddText(Form("Sigma = %.2f", hTimeBiasBottom->GetFunction("f5")->GetParameter(2)));
    pt5->Draw();


    c7->Draw();

    // Save the plots
    if (analysis_settings_file["SavePlots"]) {
        std::string output_dir = analysis_settings_file["outputDirectory"];
        std::string output_file = output_dir + "analysisPrime_" + runNumber + ".root";
        TFile *f = new TFile(output_file.c_str(), "RECREATE");
        c1->Write();
        c2->Write();
        c3->Write();
        c4->Write();
        c5->Write();
        c6->Write();
        c7->Write();
        f->Close();
    }


    if (run_root_app) app->Run();
    else LogInfo << "Info: Enable SavePlots and PlotsDisplay in AnalysisSettings.json to see plots at end of execution.\n";

    return 0;
}