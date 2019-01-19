/// @file Pr270rocessor.cpp
/// @brief Experiment processor for the PR270 Experiment at iThemba labs.
/// @author S. V. Paulauskas
/// @date January 15, 2018
//
/// @copyright Copyright (c) 2018 S. V. Paulauskas.
/// @copyright All rights reserved. Released under the Creative Commons Attribution-ShareAlike 4.0 International License
/// Used for feature-pr270dev branch  (off upstream/dev)
#include "BLMProcessor.hpp"
#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "RawEvent.hpp"
#include "RootHandler.hpp"

#include <iterator>
 
using namespace std;

double t_CI_time0=-10.0;
double t_CI_time1=-10.0;
static int t_CI_scaler0=0;  // this must be unchanged (initialised only once) until I reset it after 1 sec.
static int t_CI_scaler1=0;  // this must be unchanged (initialised only once) until I reset it after 1 sec.
int t_CI_rate0=0;
int t_CI_rate1=0;
static int cnts=0;   
static bool isFirst=false;
static long int firsttm=1.0e15;    // initialized only once in loop
static double diff=0;    // initialized only once in loop
static double Ttime=-10.0;

namespace dammIds {
    namespace experiment {
        namespace ungated {    // This is needed for PID
            const int D_CI_RATES = 2; //!< CI rates (out of full Range of 1000 Hz)
        }
    }
}

using namespace dammIds::experiment;

void BLMProcessor::DeclarePlots(void) {
    histo.DeclareHistogram1D(ungated::D_CI_RATES, SA, "CI rates (out of 1000 Hz full scale)");
}

BLMProcessor::BLMProcessor() : EventProcessor(OFFSET, RANGE, "BLMProcessor") {
    SetAssociatedTypes();

    stringstream diagname;
    diagname << Globals::get()->GetOutputPath() << Globals::get()->GetOutputFileName() << ".diag";
    diagfile.open(diagname.str().c_str());

    SetupRootOutput();
}

///Registers the ROOT tree and branches with RootHandler.
void BLMProcessor::SetupRootOutput(void) {
    tree_ = RootHandler::get()->RegisterTree("DATA", "Tree that stores some of our data");
    RootHandler::get()->RegisterBranch("DATA", "CI_time0", &t_CI_time0, "t_CI_time0/D");
    RootHandler::get()->RegisterBranch("DATA", "CI_scaler0", &t_CI_scaler0, "t_CI_scaler0/I");
    RootHandler::get()->RegisterBranch("DATA", "CI_rate0", &t_CI_rate0, "t_CI_rate0/I");
    RootHandler::get()->RegisterBranch("DATA", "CI_time1", &t_CI_time1, "t_CI_time1/D");
    RootHandler::get()->RegisterBranch("DATA", "CI_scaler1", &t_CI_scaler1, "t_CI_scaler1/I");
    RootHandler::get()->RegisterBranch("DATA", "CI_rate1", &t_CI_rate1, "t_CI_rate1/I");
}

BLMProcessor::~BLMProcessor() {
    diagfile.close();
}

void BLMProcessor::SetAssociatedTypes(void) {
    associatedTypes.insert("ci");
}

bool BLMProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    static const auto &CI_evt = event.GetSummary("ci:BLM")->GetList();

// From TreeCorrelator:
    bool PrntDiag = TreeCorrelator::get()->place("PrntDiagnostics")->status();

// --------------------- CI EVENT ---------------------------------------------------
    for (const auto &it : CI_evt) {
        auto time = it->GetTime();
        int location = it->GetChanID().GetLocation();

        if(!isFirst && (time < firsttm)) {
            firsttm=time;
            isFirst=true;
        }
        if(location==0) {
            t_CI_time0 = (time-firsttm) * 1.0e3 * Globals::get()->GetClockInSeconds();
            if(t_CI_time0>0) t_CI_scaler0 = (int)(1+t_CI_time0/1000);
            else t_CI_scaler0 = 0;
            cnts++;
        }
        if(location==1) {
            t_CI_time1 = (time-firsttm) * 1.0e3 * Globals::get()->GetClockInSeconds();
            if(t_CI_time1>0) t_CI_scaler1 = (int)(1+t_CI_time1/1000);
            else t_CI_scaler1 = 0;
            cnts++;
        }

        if(PrntDiag) diagfile<<"CI time = "<<(time-firsttm) * 1.0e3 * Globals::get()->GetClockInSeconds()<<" : cnts = "<<cnts<<endl;
        
// Counting for 1 sec. (i.e. 1000 ms):
// ----------------------------------
        static long int evt_tm0 = firsttm;    // since its 'static', it will not re-initialise
        auto evt_tm = time;
        diff = (evt_tm-evt_tm0) * 1.0e3 * Globals::get()->GetClockInSeconds();    // [in ms]
        if(diff>=1000) { 
            t_CI_rate0 = cnts; 
            histo.Plot(ungated::D_CI_RATES, t_CI_rate0);    // plots # events within 1 sec. period  
            cnts = 0;
            evt_tm0 = evt_tm;    // reset clock
        }
        if(PrntDiag) {
            diagfile<<"Time diff. = "<<diff<<endl;
            diagfile<<"CI count rate = "<<t_CI_rate0<<endl;
        }
        
        tree_->Fill();
    }
// ------------------------- Done with all the events -------------------------------
    if(PrntDiag) diagfile<<"---------------------------------------\n";
    if(!isFirst) {
        if(PrntDiag) diagfile<<"==> First time = "<<firsttm<<", (GetClockInSeconds() is "<<Globals::get()->GetClockInSeconds()<<" sec.)"<<endl;
        isFirst=true;
    }

// ---------------------------------------------------------------------------

    t_CI_time1 = -10.;
    t_CI_rate1 = 0;
    t_CI_scaler1 = 0;
    t_CI_time0 = -10.;
    t_CI_rate0 = 0;
    t_CI_scaler0 = 0;
    Ttime=0;

// --------------------------------------------------------------------------------

    EndProcess();
    return true;
}
