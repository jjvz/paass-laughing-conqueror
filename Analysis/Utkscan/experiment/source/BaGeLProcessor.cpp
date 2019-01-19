/// @file Pr270rocessor.cpp
/// @brief Experiment processor for the PR270 Experiment at iThemba labs.
/// @author S. V. Paulauskas
/// @date January 15, 2018
//
/// @copyright Copyright (c) 2018 S. V. Paulauskas.
/// @copyright All rights reserved. Released under the Creative Commons Attribution-ShareAlike 4.0 International License
/// Used for feature-pr270dev branch  (off upstream/dev)
#include "BaGeLProcessor.hpp"
#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "RawEvent.hpp"
#include "RootHandler.hpp"
#include "TROOT.h"

using namespace std;

#define TRIGGER_ONLY
#define CLOVER_ONLY

const int numSeg = 48;

double EvtWidth = 5;	// in [ms]
int trig_cnt=0;
double TrigDelay = 0;
double timeinms=0;
double t_trig_time=0;
double t_veto_time=0;
double t_tac_time=0;
double t_tac_energy=0;
double t_Ge_time[numSeg];
double t_Ge_energy[numSeg];
double t_Ge_chan[numSeg];
double t_Ge_timecoinc[numSeg];
double t_Ge_energycoinc[numSeg];
double t_Ge_chancoinc[numSeg];

static int t_trig_scaler=0;  // this must be unchanged (initialised only once) until I reset it after 1 sec.
static int t_tac_scaler=0;  // this must be unchanged (initialised only once) until I reset it after 1 sec.
static int t_veto_scaler=0;  // this must be unchanged (initialised only once) until I reset it after 1 sec.
int t_trig_rate=0;
int t_tac_rate=0;
int t_veto_rate=0;
static int cnts0=0;   
static int cnts1=0;   
static int cnts2=0;   
bool isTrigEvt = false;
bool isGeEvt = false;
static bool isFoundTrig=false;
static bool isFoundGe=false;
static bool isFirst=false;
static double firstTrigtm=8.0e12;    // [us] initialized only once in loop
static double firstGetm=8.0e12;    // [us] initialized only once in loop
static double diff=0;    // initialized only once in loop
static int EVTnum = 0;

namespace dammIds {
    namespace experiment {
            const int DD_CLOVER_EN = 0; //!< Clover vs Gamma Energy
            const int DD_CLOVERCOINC_EN = 1; //!< Clover vs Coincidence Gamma Energy
            const int D_TRIGGER_RATES = 2; //!< CI rates (out of full Range of 1000 Hz)
    }
}

using namespace dammIds::experiment;

void BaGeLProcessor::DeclarePlots(void) {
    histo.DeclareHistogram2D(DD_CLOVER_EN, SD, S6, "2D Clover Energy per Segment");
    histo.DeclareHistogram2D(DD_CLOVERCOINC_EN, SD, S6, "2D Clover Coincidence Energy per Segment");
    histo.DeclareHistogram1D(D_TRIGGER_RATES, SA, "Trigger rates");
}

BaGeLProcessor::BaGeLProcessor() : EventProcessor(OFFSET, RANGE, "BaGeLProcessor") {
    SetAssociatedTypes();

    stringstream diagname;
    diagname << Globals::get()->GetOutputPath() << Globals::get()->GetOutputFileName() << ".diag";
    diagfile.open(diagname.str().c_str());

    SetupRootOutput();
}

///Registers the ROOT tree and branches with RootHandler.
void BaGeLProcessor::SetupRootOutput(void) {
    tree_ = RootHandler::get()->RegisterTree("DATA", "Tree that stores some of our data");
#ifndef CLOVER_ONLY
    RootHandler::get()->RegisterBranch("DATA", "trig_time", &t_trig_time, "t_trig_time/D");
    RootHandler::get()->RegisterBranch("DATA", "tac_time", &t_tac_time, "t_tac_time/D");
    RootHandler::get()->RegisterBranch("DATA", "trig_scaler", &t_trig_scaler, "t_trig_scaler/I");
    RootHandler::get()->RegisterBranch("DATA", "tac_scaler", &t_tac_scaler, "t_tac_scaler/I");
    RootHandler::get()->RegisterBranch("DATA", "trig_rate", &t_trig_rate, "t_trig_rate/I");
    RootHandler::get()->RegisterBranch("DATA", "tac_rate", &t_tac_rate, "t_tac_rate/I");
    RootHandler::get()->RegisterBranch("DATA", "tac_energy", &t_tac_energy, "t_tac_energy/D");
    RootHandler::get()->RegisterBranch("DATA", "Evtnum", &EVTnum, "EVTnum/I");
    RootHandler::get()->RegisterBranch("DATA", "trig_cnt", &trig_cnt, "trig_cnt/I");
#endif
#ifndef TRIGGER_ONLY
    RootHandler::get()->RegisterBranch("DATA", "Ge_time", t_Ge_time, "t_Ge_time[48]/D");
    RootHandler::get()->RegisterBranch("DATA", "Ge_rawEnergy", t_Ge_chan, "t_Ge_chan[48]/D");
    RootHandler::get()->RegisterBranch("DATA", "Ge_calEnergy", t_Ge_energy, "t_Ge_energy[48]/D");
    RootHandler::get()->RegisterBranch("DATA", "Ge_timeCoinc", t_Ge_timecoinc, "t_Ge_timecoinc[48]/D");
    RootHandler::get()->RegisterBranch("DATA", "Ge_rawEnergyCoinc", t_Ge_chancoinc, "t_Ge_chancoinc[48]/D");
    RootHandler::get()->RegisterBranch("DATA", "Ge_calEnergyCoinc", t_Ge_energycoinc, "t_Ge_energycoinc[48]/D");

#endif
}

BaGeLProcessor::~BaGeLProcessor() {
    diagfile.close();
}

void BaGeLProcessor::SetAssociatedTypes(void) {
//    associatedTypes.insert("logic");
    associatedTypes.insert("ge");
}

// ----------------------------------------------------------------------------------
// --------------------- PROCESS ----------------------------------------------------
// ----------------------------------------------------------------------------------
bool BaGeLProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

//    static const auto &trig_evt = event.GetSummary("logic:trigger")->GetList();
    static const auto &Ge_evt = event.GetSummary("ge:clover")->GetList();

// From TreeCorrelator:
    bool PrntDiag = TreeCorrelator::get()->place("PrntDiagnostics")->status();
//    double clock2sec = 1.0e6 * Globals::get()->GetFilterClockInSeconds(); // now in [us]
    double clock2sec = 1.0e6 * Globals::get()->GetClockInSeconds();   // now in [us]

//    firstTrigtm = firstTrigtm*clock2sec;

// ----------------------------------------------------------------------------------
// --------------------- TRIGGER EVENT ----------------------------------------------
// ----------------------------------------------------------------------------------
/*
    for (const auto &it : trig_evt) {
        int location = it->GetChanID().GetLocation();
        double energy = it->GetEnergy();
        double time = it->GetTime();

        if(!isFirst && (time < firstTrigtm)) {
            firstTrigtm=time;
            isFoundTrig=true;
        }
        
        if(location==0) {	// k600 trigger logic signal
            t_trig_time = (time-firstTrigtm)*1.0e3*Globals::get()->GetClockInSeconds();
            if(t_trig_time>0) t_trig_scaler = (int)(1+t_trig_time/1000);
            else t_trig_scaler = 0;
            cnts0++;
            EVTnum++;
        }
        if(location==1) {	// TAC output
            t_tac_time = (time-firstTrigtm)*1.0e3*Globals::get()->GetClockInSeconds();
            t_tac_energy = energy;
            if(t_tac_time>0) t_tac_scaler = (int)(1+t_tac_time/1000);
            else t_tac_scaler = 0;
            cnts1++;
        }
        if(location==2) {	// Veto output
            t_veto_time = (time-firstTrigtm)*1.0e3*Globals::get()->GetClockInSeconds();
            if(t_veto_time>0) t_veto_scaler = (int)(1+t_veto_time/1000);
            else t_veto_scaler = 0;
            cnts2++;
        }
        
        if(PrntDiag) {
          diagfile<<"first time = "<<firstTrigtm<<" clockticks"<<endl;
          diagfile<<"time = "<<time<<" clockticks"<<endl;
          diagfile<<"trigger time = "<<t_trig_time<<" msec."<<endl;
          diagfile<<"tac time = "<<t_tac_time<<" msec."<<endl;
        }
        
// Counting for 1 sec. (i.e. 1000 ms):
// ----------------------------------
        static double evt_tm0 = firstTrigtm;    // since its 'static', it will not re-initialise
        double evt_tm = time;
        diff = (evt_tm-evt_tm0) * 1.0e3 * Globals::get()->GetClockInSeconds();    // [in ms]
        if(diff>=1000) { 
            t_trig_rate = cnts0; 
            t_tac_rate = cnts1; 
            t_veto_rate = cnts2; 
            histo.Plot(D_TRIGGER_RATES, t_trig_rate);    // plots # events within 1 sec. period  
            cnts0 = 0;
            cnts1 = 0;
            cnts2 = 0;
            evt_tm0 = evt_tm;    // reset clock
        }
        if(PrntDiag) {
            diagfile<<"Time diff. (evt time - first time) = "<<diff<<endl;
            diagfile<<"trigger count rates = "<<t_trig_rate<<"\t "<<t_tac_rate<<endl;
        }
        isTrigEvt=true;
    }
//    if(isTrigEvt) tree_->Fill();
*/
// --------------------------------------------------------------------------------------
// --------------------- CLOVER EVENT ---------------------------------------------------
// --------------------------------------------------------------------------------------
static int cntr=0;
    for (const auto &it : Ge_evt) {
        double chan = it->GetEnergy();
        double energy = it->GetCalibratedEnergy();
        double time = it->GetFilterTime();
//        double time = it->GetTime();
        int location = it->GetChanID().GetLocation();

//        if(isFoundTrig && isFoundGe) TrigDelay = (firstTrigtm - firstGetm)*clock2sec;   /// positive time diff. between trigger and clovers, in [us]

//        timeinms = time*clock2sec - firstTrigtm;  // now in [us]
        timeinms = time*clock2sec;   // now in [us]
//        timeinms = time;

//        if(PrntDiag) diagfile<<"location: "<<location<<" \t|| energy: "<< chan <<"\t|| time: "<<std::setprecision (15) <<timeinms<<" usec. || trigger cnts = "<<cnts0<<endl;        
        if(PrntDiag && location<15) diagfile<<std::setprecision (15) <<timeinms<<"\t"<<cntr<<endl;        

#ifndef CLOVER_ONLY
        if(isFoundGe && isFoundTrig) TrigDelay = firstTrigtm - firstGetm;
        
        if(location==0) {	// k600 trigger logic signal

            if(!isFoundTrig && (timeinms < firstTrigtm)) {
               firstTrigtm = timeinms;
               isFoundTrig = true;
            }

            t_trig_time = timeinms - firstTrigtm;
            if(t_trig_time>0) t_trig_scaler = (int)(1+t_trig_time/1000);
            else t_trig_scaler = 0;
            cnts0++;
            EVTnum++;
            isTrigEvt=true;
            trig_cnt++;

            if(PrntDiag) {
//              diagfile<<"first trigger time = "<< std::setprecision (15) <<firstTrigtm<<" usec."<<endl;
//              diagfile<<"time = "<< std::setprecision (15) <<time<<" clockticks"<<endl;
              diagfile<<"trigger time = "<< std::setprecision (15) <<t_trig_time<<" usec."<<endl;
            }
        }

        if(location==1) {	// TAC output
            t_tac_time = timeinms - firstTrigtm;
            t_tac_energy = energy;
            if(t_tac_time>0) t_tac_scaler = (int)(1+t_tac_time/1000);
            else t_tac_scaler = 0;
            cnts1++;
        }
        if(location==2) {	// Veto output
            t_veto_time = timeinms - firstTrigtm;
            if(t_veto_time>0) t_veto_scaler = (int)(1+t_veto_time/1000);
            else t_veto_scaler = 0;
            cnts2++;
        }
#endif

// -------------------------------------------------------------------------------------
#ifndef TRIGGER_ONLY

        for (int i=3;i<numSeg+3;i++) {
            if(location==i) {
                if(!isFoundGe && (timeinms < firstGetm)) {
		            firstGetm=timeinms;
		            isFoundGe=true;
	            }
                t_Ge_energy[i-3] = energy;
                t_Ge_chan[i-3] = chan;
                t_Ge_time[i-3] = timeinms - firstTrigtm + TrigDelay;
                if(abs(timeinms - t_trig_time) < EvtWidth) {
                    t_Ge_energycoinc[i-3] = energy;
                    t_Ge_chancoinc[i-3] = chan;
                    t_Ge_timecoinc[i-3] = timeinms - firstTrigtm + TrigDelay;
                    histo.Plot(DD_CLOVERCOINC_EN, 0.25*t_Ge_chancoinc[i-3], i-3);
                }
                histo.Plot(DD_CLOVER_EN, 0.25*t_Ge_chan[i-3], i-3);

                isGeEvt = true;
                if(PrntDiag) diagfile<<"Ge time["<<i<<"] = "<< std::setprecision (15) <<t_Ge_time[i-3]<<" usec. : cnts = "<<cnts0<<endl;        
	         }
        }
#endif
// --------------------- END OF CLOVER EVENT ---------------------------------------------------
// --------------------------------------------------------------------------------------

    }

// --------------------------------------------------------------------------------------
// I fill the root tree here, i.e. after all the signals within one Event Width (e.g. 1 us)
// has been read and stored into variables.
// Now if I fill the tree here, those varables will be stored in the root event tree, 
// and I can clear the variables.
 
    tree_->Fill();

// Note, the above method ignores any multiplicity within one EventWidth, unless I push them 
// into arrays or vectors...
    
// ------------------------- Done with all the events -------------------------------
    cntr++;
//    if(PrntDiag) diagfile<<"--------- DONE WITH EVENTWIDTH #"<<cntr<<" : TIME = "<< std::setprecision (15) <<timeinms<<" usec. ---------\n";
//    if(PrntDiag) diagfile<<"------------------------------------------------------------------------------------------------------- # "<<cntr<<endl;
    if(!isFirst && isFoundTrig) {
        if(PrntDiag) diagfile<<"==> First trigger time = "<< std::setprecision (15) <<firstTrigtm<<" usec., (GetClockInSeconds() is "<<Globals::get()->GetClockInSeconds()<<" sec/clocktick)"<<endl;
        isFirst=true;
    }

// ---------------------------------------------------------------------------
// --------------------- ZERO VARIABLES AFTER FILL ---------------------------
// ---------------------------------------------------------------------------

    trig_cnt=0;
    t_trig_time = 0;
    t_tac_time = 0;
    t_tac_energy = 0;
    t_trig_rate = 0;
    t_tac_rate = 0;
    t_trig_scaler = 0;
    t_tac_scaler = 0;

//    memset(t_trig_time,0,sizeof(t_trig_time)/sizeof(double));
    memset(t_Ge_energy,0,sizeof(t_Ge_energy)/sizeof(double));
    memset(t_Ge_chan,0,sizeof(t_Ge_chan)/sizeof(double));
    memset(t_Ge_time,0,sizeof(t_Ge_time)/sizeof(double));
    memset(t_Ge_energycoinc,0,sizeof(t_Ge_energycoinc)/sizeof(double));
    memset(t_Ge_chancoinc,0,sizeof(t_Ge_chancoinc)/sizeof(double));
    memset(t_Ge_timecoinc,0,sizeof(t_Ge_timecoinc)/sizeof(double));
    isTrigEvt = false;
    isGeEvt = false;
  
// ----------------------------------------------------------------------------------

    EndProcess();
    return true;
// --------------------- END OF PROCESS ---------------------------------------------
// ----------------------------------------------------------------------------------
}

