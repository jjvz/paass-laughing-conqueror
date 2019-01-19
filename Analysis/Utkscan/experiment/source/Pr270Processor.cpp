/// @file Pr270rocessor.cpp
/// @brief Experiment processor for the PR270 Experiment at iThemba labs.
/// @author S. V. Paulauskas
/// @date January 15, 2018
//
/// @copyright Copyright (c) 2018 S. V. Paulauskas.
/// @copyright All rights reserved. Released under the Creative Commons Attribution-ShareAlike 4.0 International License
/// Used for feature-pr270dev branch  (off upstream/dev)
#include "Pr270Processor.hpp"
#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "RawEvent.hpp"
#include "RootHandler.hpp"

#include <iterator>
 
using namespace std;

struct Kpol {
    double t_siL_ch;
    double t_siL_energy;
    double t_siL_time;
    double t_siR_ch;
    double t_siR_energy;
    double t_siR_time;
};
static Kpol kpol_;

struct Telescope {
    double  t_E_ch;
    double  t_E_energy;
    double t_dE_ch;
    double t_dE_energy;
    double  t_E_time;
    double t_dE_time;
    double t_E_tot;
};
static Telescope telescopeL_;
static Telescope telescopeR_;

/*
 * struct ELENERGY {
    vector<double> *t_EL_energy=nullptr;
    double t_EL_time;
};
static ELENERGY EL_energy_;
*/

// root tree variables:
//std::vector<double> t_ELv_energy;
double t_E_det=-1;
double t_dE_det = -1;
double t_pulser_time=-9999.0;
double t_CI_time=-10.0;
double t_massfL=-10.;
double t_massfR=-10.0; 
bool t_isEdE_L = false;
bool t_isEdE_R = false;
bool t_isPulserL = false;
bool t_isPulserR = false;
bool L_gated = false;
bool R_gated = false;
static int t_CI_scaler=0;  // this must be unchanged (initialised only once) until I reset it after 1 sec.
int t_CI_rate=0;
int t_polu=0;
int t_pold=0;
static int t_evtno=0;    // initialized only once in loop

//static double t1;
static int LU=0;
static int LD=0;
static int RU=0;
static int RD=0;
static int cnts=0;   
static bool isFirst=false;
static bool isFound=false;
static bool isUnPol=true;
static long int firsttm=1.0e15;    // initialized only once in loop
//double evt_tm=0;    // initialized only once in loop
double mf=1.73;    // mass function exponent, from literature 
static double diff=0;    // initialized only once in loop
static double Ttime=-10.0;
double trig_timeL=0.;    // first trigger time of dE left
double trig_timeR=0.;    // first trigger time of dE right
double evtWidth=1.0e-3;    // dE-E event width in [ms]

std::vector<double> silE, silT, nailE, nailT, tell_siE, tell_naiE, dts, dte;         
std::vector<double> sirE, sirT, nairE, nairT, telr_siE, telr_naiE, pulser_t;         

namespace dammIds {
    namespace experiment {
        namespace ungated {    // This is needed for PID
            const int DD_TELESCOPE_L = 0; //!< Ungated dE vs. E for Telescope L
            const int DD_TELESCOPE_R = 1; //!< Ungated dE vs. E for Telescope R
            const int D_CI_RATES = 2; //!< CI rates (out of full Range of 1000 Hz)
            const int D_PULSER_L = 3; //!< NaI energy Left - pulser only
            const int D_PULSER_R = 4; //!< NaI energy Right - pulser only
        }
    }
}

using namespace dammIds::experiment;

void Pr270Processor::DeclarePlots(void) {
    histo.DeclareHistogram1D(ungated::D_CI_RATES, SA, "CI rates (out of 1000 Hz full scale)");
    histo.DeclareHistogram1D(ungated::D_PULSER_L, SE, "NaI Energy Left - Pulser only");
    histo.DeclareHistogram1D(ungated::D_PULSER_R, SE, "NaI Energy Right - Pulser only");
    histo.DeclareHistogram2D(ungated::DD_TELESCOPE_L, SB, S9, "Ungated dE vs. E - Telescope L");
    histo.DeclareHistogram2D(ungated::DD_TELESCOPE_R, SB, S9, "Ungated dE vs. E - Telescope R");
}

Pr270Processor::Pr270Processor(const unsigned int &k_linepol) : EventProcessor(OFFSET, RANGE, "Pr270PRocessor") {
//    isK_linepol_ = k_linepol;
    SetAssociatedTypes();

    stringstream diagname;
    diagname << Globals::get()->GetOutputPath() << Globals::get()->GetOutputFileName() << ".diag";
    diagfile.open(diagname.str().c_str());

    stringstream polname;
    polname << Globals::get()->GetOutputPath() << Globals::get()->GetOutputFileName() << ".pol";
    polfile.open(polname.str().c_str());

    SetupRootOutput();
}

///Registers the ROOT tree and branches with RootHandler.
void Pr270Processor::SetupRootOutput(void) {
    tree_ = RootHandler::get()->RegisterTree("DATA", "Tree that stores some of our data");

    ///Registers a branch with the provided tree.
//    void RegisterBranch(const std::string &treeName, const std::string &name, void *address, const std::string &leaflist);
    ///@param[in] treeName : The name of the tree that they want to add a branch to.
    ///@param[in] name : The name of the branch that they're adding
    ///@param[in] address : A pointer to the memory address for the object we're adding to the tree
    ///@param[in] leaflist : The leaf definition for the branch.

    RootHandler::get()->RegisterBranch("DATA", "TelescopeR", &telescopeR_, "E_ch/D:E_energy/D:dE_ch/D:dE_energy:E_time:dE_time:E_tot");
    RootHandler::get()->RegisterBranch("DATA", "TelescopeL", &telescopeL_, "E_ch/D:E_energy/D:dE_ch/D:dE_energy:E_time:dE_time:E_tot");
//    RootHandler::get()->RegisterBranch("DATA", "ELv_energy", &t_ELv_energy, "t_ELv_energy/D");
//    RootHandler::get()->RegisterBranch("DATA", "EL", &EL_energy_,"EL_energy/D:EL_time/D");
    RootHandler::get()->RegisterBranch("DATA", "E_det", &t_E_det, "t_E_det/I");
    RootHandler::get()->RegisterBranch("DATA", "dE_det", &t_dE_det, "t_dE_det/I");
    RootHandler::get()->RegisterBranch("DATA", "pulser_time", &t_pulser_time, "t_pulser_time/D");
    RootHandler::get()->RegisterBranch("DATA", "CI_time", &t_CI_time, "t_CI_time/D");
    RootHandler::get()->RegisterBranch("DATA", "CI_scaler", &t_CI_scaler, "t_CI_scaler/I");
    RootHandler::get()->RegisterBranch("DATA", "CI_rate", &t_CI_rate, "t_CI_rate/I");
    RootHandler::get()->RegisterBranch("DATA", "isEdE_L", &t_isEdE_L, "t_isEdE_L/O");
    RootHandler::get()->RegisterBranch("DATA", "isEdE_R", &t_isEdE_R, "t_isEdE_R/O");
    RootHandler::get()->RegisterBranch("DATA", "isPulserL", &t_isPulserL, "t_isPulserL/O");
    RootHandler::get()->RegisterBranch("DATA", "isPulserR", &t_isPulserR, "t_isPulserR/O");
    RootHandler::get()->RegisterBranch("DATA", "evtno", &t_evtno, "t_evtno/I");
    RootHandler::get()->RegisterBranch("DATA", "massfL", &t_massfL, "t_massfL/D");
    RootHandler::get()->RegisterBranch("DATA", "massfR", &t_massfR, "t_massfR/D");

    RootHandler::get()->RegisterBranch("DATA", "polu", &t_polu, "t_polu/I");
    RootHandler::get()->RegisterBranch("DATA", "pold", &t_pold, "t_pold/I");

    RootHandler::get()->RegisterBranch("DATA", "Kpol", &kpol_, "siL_energy/D:siL_time/D:siR_energy/D:siR_time/D");
}

Pr270Processor::~Pr270Processor() {
    diagfile.close();
    polfile.close();
}

void Pr270Processor::SetAssociatedTypes(void) {
    associatedTypes.insert("tel");
//    associatedTypes.insert("pulser");
    associatedTypes.insert("si");
    associatedTypes.insert("logic");
    associatedTypes.insert("ci");
}

bool Pr270Processor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    static const auto &pol_evt = event.GetSummary("logic:pol")->GetList();
    bool PrntDiag = TreeCorrelator::get()->place("PrntDiagnostics")->status();

// ----------------------------------------------------------------------------------
// --------------------- POL UP/DWN EVENT -------------------------------------------
// ----------------------------------------------------------------------------------
    for (const auto &it : pol_evt) {
        auto location = it->GetChanID().GetLocation();
//        auto energy = it->GetCalibratedEnergy();
        auto time = it->GetTime();
        double dt_up=0;

        TreeCorrelator::get()->place("PolUp")->deactivate(time);
        TreeCorrelator::get()->place("PolDwn")->deactivate(time);

        if (location == 0 ) {
            TreeCorrelator::get()->place("PolUp")->activate(time);
//            TreeCorrelator::get()->place("PolDwn")->deactivate(time);
//            double dt_start = time - TreeCorrelator::get()->place(place)->secondlast().time;
            isUnPol=false;
        }
        else if (location == 1 ) {
            TreeCorrelator::get()->place("PolDwn")->activate(time);
//            TreeCorrelator::get()->place("PolUp")->deactivate(time);
//            double dt_stop = time - TreeCorrelator::get()->place(place)->secondlast().time;
            dt_up = time - TreeCorrelator::get()->place("logic_pol_0")->last().time;
            isUnPol=false;
        }

        if(PrntDiag) {
            diagfile<<"Pol U/D evt "<<location<<", time = "<<dt_up * 1.0e3 * Globals::get()->GetClockInSeconds()<<endl;
            diagfile<<"Time beam was UP: "<<dt_up<<endl;
        }
    }
    return true;
}

bool Pr270Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    static double kPolrunTimeSecs = 0;
    bool is1sec = false;

    static const auto &T_evt = event.GetSummary("tel:E")->GetList();
    static const auto &kpol_evt = event.GetSummary("si:kpol")->GetList();
//    static const auto &puls_evt = event.GetSummary("pulser:E")->GetList();
    static const auto &CI_evt = event.GetSummary("ci:currentIntegrator")->GetList();

// From TreeCorrelator:
    bool isK_linepol_ = TreeCorrelator::get()->place("isK_linepol")->status();

    if ( isK_linepol_ ) {
        L_gated = TreeCorrelator::get()->place("SiLgate")->status();
        R_gated = TreeCorrelator::get()->place("SiRgate")->status();
    }
    else {
        L_gated = TreeCorrelator::get()->place("TelLgate")->status();
        R_gated = TreeCorrelator::get()->place("TelRgate")->status();
    }

//    bool isTelescopeL = TreeCorrelator::get()->place("TelescopeL")->status();
//    bool isTelescopeR = TreeCorrelator::get()->place("TelescopeR")->status();
    bool UpPolarized = TreeCorrelator::get()->place("PolUp")->status();
    bool DwnPolarized = TreeCorrelator::get()->place("PolDwn")->status();
    if(!UpPolarized && !DwnPolarized) isUnPol=true;

    bool PrntDiag = TreeCorrelator::get()->place("PrntDiagnostics")->status();
    bool PrntPol = TreeCorrelator::get()->place("PrntPolarisation")->status();

    
    if(PrntDiag)
        diagfile<<"Pol-up status: "<<UpPolarized<<", Pol-dn status: "<<DwnPolarized<<endl;

/*    
// ----------------------------------------------------------------------------------
// --------------------- PULSER EVENT ---------------------------------------------------
// ----------------------------------------------------------------------------------
    for (const auto &it : puls_evt) {
//        auto energy = it->GetCalibratedEnergy();
        auto time = it->GetTime();

        if(!isFirst && (time < firsttm)) {
            firsttm=time;
            isFound=true;
        }
        t_pulser_time = (time-firsttm)*1.0e3*Globals::get()->GetClockInSeconds();
        pulser_t.push_back(t_pulser_time);
        t_isPulserL=true;

        if(PrntDiag) {
          diagfile<<"pulser time = "<<(time-firsttm)*1.0e3*Globals::get()->GetClockInSeconds()<<" msec."<<endl;
          diagfile<<"time = "<<time<<" c.t., firsttime = "<<firsttm<<endl;
        }
        tree_->Fill();
    }
// -----------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------

    for(int pp=0;pp<(int)pulser_t.size();pp++) {
        for(int pn=0;pn<(int)nailT.size();pn++) {
            dts.push_back( abs(nailT.at(pn)-pulser_t.at(pp)) );
            dte.push_back( nailE.at(pn) );
//            if( dts.at(pp)<0.0015 ) { histo.Plot(ungated::D_PULSER_L, dte.at(pp)); }
            histo.Plot(ungated::D_PULSER_L, nailE.at(pn));
        }
     }

       if( dts.size()>0 ) {
            auto dtmin = min_element(begin(dts), end(dts));
            int dtindex = distance(dts.begin(), dtmin);
//            std::cout << "\nminimum value is "<<*dtmin << ", found at index position "<< dtindex<< std::endl;
//            histo.Plot(ungated::D_PULSER_L, dte.at(dtindex));
            t_isPulserL = true;
        }
        dts.clear();
        dte.clear(); 
        for(int pp=0;pp<(int)nairT.size();pp++) {
            dts.push_back( abs(nairT.at(pp)-t_pulser_time) );
            dte.push_back( nairE.at(pp) );
        }
        if( dts.size()>0 ) {
            auto dtmin = min_element(begin(dts), end(dts));
            int dtindex = distance(dts.begin(), dtmin);
            histo.Plot(ungated::D_PULSER_R, dte.at(dtindex));
            t_isPulserR = true;
        }
        dts.clear();
        dte.clear();
*/
// ----------------------------------------------------------------------------------
// ---------------------- TELESCOPE EVENT ---------------------------------------------------
// ----------------------------------------------------------------------------------
    for (const auto &it : T_evt) {
        int location = it->GetChanID().GetLocation();
        double chan = it->GetEnergy();
        double energy = it->GetCalibratedEnergy();
        double time = it->GetTime();

        if(!isFirst && (time < firsttm)) {
            firsttm=time;
            isFound=true;
        }
        
        Ttime = (time-firsttm) * 1.0e3 * Globals::get()->GetClockInSeconds();   // time in [ms] from first event time

        if (location == 0) {    // Left dE event    
            telescopeL_.t_dE_ch = chan;
            telescopeL_.t_dE_energy = energy;
            telescopeL_.t_dE_time = Ttime;
            silE.push_back(energy);
            silT.push_back(Ttime);
         }
        if (location == 1) {    // Right dE event    
            telescopeR_.t_dE_ch = chan;
            telescopeR_.t_dE_energy = energy;
            telescopeR_.t_dE_time = Ttime;
            sirE.push_back(energy);
            sirT.push_back(Ttime);
        }
        if (location == 3) {    // Left E event    
            telescopeL_.t_E_ch = chan;
            telescopeL_.t_E_energy = energy;
            telescopeL_.t_E_time = Ttime;
            if( pulser_t.size()>0 ) {
              if(abs(Ttime-pulser_t.back())<0.0015) {
                    histo.Plot(ungated::D_PULSER_L, energy);
              }
            }
            nailT.push_back(Ttime);
            nailE.push_back(energy);
            if( silE.size()>0 ) {
              if( abs(Ttime-silT.back())<evtWidth ) { 
                tell_siE.push_back(silE.back());
                tell_naiE.push_back(energy);
                t_isEdE_L=true;
              }
            }
        }

        if (location == 4) {    // Right E event    
            telescopeR_.t_E_ch = chan;
            telescopeR_.t_E_energy = energy;
            telescopeR_.t_E_time = Ttime;
            if( pulser_t.size()>0 ) {
              if(abs(Ttime-pulser_t.back())<0.0015) {
                    histo.Plot(ungated::D_PULSER_R, energy);
              }
            }
            nairT.push_back(Ttime);
            nairE.push_back(energy);
            if( sirE.size()>0 ) {
              if( abs(Ttime-sirT.back())<evtWidth ) { 
                telr_siE.push_back(sirE.back());
                telr_naiE.push_back(energy);
                t_isEdE_R=true;
              }
            }
        }

        if (location == 2) {    // pulser event
            t_pulser_time = Ttime;
            pulser_t.push_back(t_pulser_time);
            t_isPulserL = true;
        }
        
        t_E_det = location;

        if(PrntDiag) {
            diagfile<<"Detector "<<location<<", energy = "<<energy<<endl;
            diagfile<<"dt = "<<Ttime<<" msec."<<endl;
        }

        if(location==0 || location==2) telescopeL_.t_E_tot += energy;
        if(location==1 || location==3) telescopeR_.t_E_tot += energy;
        
        for(unsigned int i=0;i<tell_siE.size();i++) { 
            histo.Plot(ungated::DD_TELESCOPE_L, 0.25*tell_naiE.at(i), 10.*tell_siE.at(i));
            t_massfL = pow((tell_siE.at(i)+tell_naiE.at(i)),mf) - pow(tell_naiE.at(i),mf);
        }
        for(unsigned int i=0;i<telr_siE.size();i++) { 
            histo.Plot(ungated::DD_TELESCOPE_R, 0.25*telr_naiE.at(i), 10.*telr_siE.at(i));
            t_massfR = pow((telr_siE.at(i)+telr_naiE.at(i)),mf) - pow(telr_naiE.at(i),mf);
        }
//        if(isTelescopeL) { histo.Plot(ungated::DD_TELESCOPE_L, telescopeL_.t_E_energy, telescopeL_.t_dE_energy); }
//        if(isTelescopeR) { histo.Plot(ungated::DD_TELESCOPE_R, telescopeR_.t_E_energy, telescopeR_.t_dE_energy); }
 
//        tree_->Fill();
    }

// ----------------------------------------------------------------------------------
// --------------------- CI EVENT ---------------------------------------------------
// ----------------------------------------------------------------------------------

    for (const auto &it : CI_evt) {
        auto time = it->GetTime();

        if(!isFirst && (time < firsttm)) {
            firsttm=time;
            isFound=true;
        }
        t_CI_time = (time-firsttm) * 1.0e3 * Globals::get()->GetClockInSeconds();
        if(t_CI_time>0) t_CI_scaler = (int)(1+t_CI_time/1000);
        else t_CI_scaler = 0;
        cnts++;

        if(PrntDiag) diagfile<<"CI time = "<<(time-firsttm) * 1.0e3 * Globals::get()->GetClockInSeconds()<<" : cnts = "<<cnts<<endl;
        
// Counting for 1 sec. (i.e. 1000 ms):
// ----------------------------------
        static long int evt_tm0 = firsttm;    // since its 'static', it will not re-initialise
        auto evt_tm = time;
        diff = (evt_tm-evt_tm0) * 1.0e3 * Globals::get()->GetClockInSeconds();    // [in ms]
        if(diff>=1000) { 
            t_CI_rate = cnts; 
            histo.Plot(ungated::D_CI_RATES, t_CI_rate);    // plots # events within 1 sec. period  
            cnts = 0;
            evt_tm0 = evt_tm;    // reset clock
        }
// ----------------------------------
        if(PrntDiag) {
            diagfile<<"Time diff. = "<<diff<<endl;
            diagfile<<"CI count rate = "<<t_CI_rate<<endl;
        }
        
//        tree_->Fill();
    }
// ----------------------------------------------------------------------------------
// --------------------- K-POL EVENT ---------------------------------------------------
// ----------------------------------------------------------------------------------
    for (const auto &it : kpol_evt) {
        auto location = it->GetChanID().GetLocation();
        auto chan = it->GetEnergy();
        auto energy = it->GetCalibratedEnergy();   
        auto time = it->GetTime();

        if(!isFirst && (time < firsttm)) {
            firsttm=time;
            isFound=true;
        }

        kPolrunTimeSecs = (time - firsttm) * Globals::get()->GetClockInSeconds();    // [in sec.] e.g. (4.2e10 - 4.0e10)[clockticks] * 8e-9 [s/clocktick] = 0.2e10 * 8e-9 = 16 sec.
        static long int evt_tm0 = firsttm;    // since its 'static', it will not re-initialise
        auto evt_tm = time;
        diff = (evt_tm-evt_tm0) * 1.0e3 * Globals::get()->GetClockInSeconds();    // [in ms]
        if(diff>=1000) { 
            is1sec=true; 
            evt_tm0=evt_tm;    //reset clock
        }

        if(PrntDiag) {
            diagfile<<"kpol si "<<location<<", time = "<<(time-firsttm) * 1.0e3 * Globals::get()->GetClockInSeconds();
            diagfile<<"\t energy = "<<energy<<endl;
            diagfile<<"Pol-up status: "<<UpPolarized<<", Pol-dn status: "<<DwnPolarized<<endl;
        }

        if (location == 0) {    // k-pol Left event    
            kpol_.t_siL_ch = chan;
            kpol_.t_siL_energy = energy;
            kpol_.t_siL_time = (time-firsttm) * 1.0e3 * Globals::get()->GetClockInSeconds();
        }

        if (location == 1) {    // k-pol Right event    
            kpol_.t_siR_ch = chan;
            kpol_.t_siR_energy = energy;
            kpol_.t_siR_time = (time-firsttm) * 1.0e3 * Globals::get()->GetClockInSeconds();
        }
    }
// ----------------------------------------------------------------------------------    
// ------------------------- Done with all the events -------------------------------
// ----------------------------------------------------------------------------------    
    if(PrntDiag) diagfile<<"---------------------------------------\n";

    if(t_isEdE_L || t_isEdE_R) {
        t_evtno++;
        if(PrntDiag) 
            diagfile<<"**** Proper dE-E event no.: "<<t_evtno<<endl<<endl;
    }

// ---------------------------------------------------------------------------
    if(!isFirst && isFound) {
        if(PrntDiag) diagfile<<"==> First time = "<<firsttm<<", (GetClockInSeconds() is "<<Globals::get()->GetClockInSeconds()<<" sec.)"<<endl;
        isFirst=true;
    }

// ----------- Calculate Polarisation -----------------------------------------    
    if(UpPolarized) {    //There will always be a beam pol up/dwn status, except unpol case 'else' below
        t_polu=1;
        t_pold=0;
    }
    else if(DwnPolarized) {
        t_polu=0;
        t_pold=1;
    }
    else {                // lets say the ion source gives 0 when unpolarised beam...
        t_polu=0;
        t_pold=0;
    }

// ------------For all Pol measurements:-----------------------------------
// ------------------------------------------------------------------------
    if( L_gated ) {
        if (UpPolarized ) LU++;    // if in elstic peak gate set in Config.xml
        else if (DwnPolarized)    LD++;
        else if (isUnPol) { 
            LU++;
            LD++;
        }
    }
    if( R_gated ) {
        if (UpPolarized ) RU++;    // if in elstic peak gate set in Config.xml
        else if (DwnPolarized)    RD++;
        else if (isUnPol) { 
            RU++;
            RD++;
        }
    }
//        static long int evt_tm0 = firsttm;    // since its 'static', it will not re-initialise
//        auto evt_tm = time;
//        diff = (evt_tm-evt_tm0) * 1.0e3 * Globals::get()->GetClockInSeconds();    // [in ms]
//        if(diff>=1000) { 
//            is1sec=true; 
//            evt_tm0=evt_tm;    //reset clock
//        }

    if(is1sec && PrntPol) { 
//        if(PrntPol) { 
        polfile << isK_linepol_ <<"\t"<<isUnPol <<"\t"<< LU <<"\t"<< LD <<"\t"<< RU <<"\t"<< RD <<"\t"<< kPolrunTimeSecs <<"\n";    // writes the time stamp in s.
//            else polfile << isUnPol <<"\t"<< (int)(LU/UNPOL) <<"\t"<< (int)(LD/UNPOL) <<"\t"<< RU <<"\t"<< RD <<"\t"<< kPolrunTimeSecs <<"\n";    // writes the time stamp in s.
        polfile << std::flush;    // flushes remaining buffer content to file
        polfile.clear();
    } 
// ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------

    tree_->Fill();
// Note: if I fill the TTree in other loops instead of once after the eventWidth, then other leaves will also 
// be filled again with their last values, giving unreal, repeated, residual counts!! So, choose when is best time 
// to Fill the TTree to ensure every ROOT event row assocciates only those affected (and related) leaves. 

// Zero variables:
// ---------------
    telescopeL_.t_E_energy = 0.0;
    telescopeR_.t_E_energy = 0.0;
    telescopeL_.t_dE_energy = 0.0;
    telescopeR_.t_dE_energy = 0.0;
    telescopeL_.t_E_time = 0.0;
    telescopeR_.t_E_time = 0.0;
    telescopeL_.t_dE_time = 0.0;
    telescopeR_.t_dE_time = 0.0;
    telescopeL_.t_E_tot = 0.0;
    telescopeR_.t_E_tot = 0.0;
//    EL_energy_.t_EL_energy.push_back(100.0);
//    EL_energy_.t_EL_time = 0.10;
//    t_ELv_energy.clear();
    t_dE_det = -1;
    t_E_det = -1;
//    t_pulser_time = 0.0;
    kpol_.t_siL_energy = -10.0;
    kpol_.t_siR_energy = -10.0;
    kpol_.t_siL_time = -10.0;
    kpol_.t_siR_time = -10.0;
    t_CI_time = -10.;
    t_CI_rate = 0;
    t_CI_scaler = 0;
    t_isEdE_L=false;
    t_isEdE_R=false;
    t_isPulserL=false;
    t_isPulserR=false;
    Ttime=0;

    silE.clear();
    silT.clear();
    nailT.clear();
    nailE.clear();
    tell_siE.clear();
    tell_naiE.clear();         
    sirE.clear();
    sirT.clear();
    nairT.clear();
    nairE.clear();
    telr_siE.clear();
    telr_naiE.clear();         
//    pulser_t.clear();
    dts.clear();
    dte.clear();

// --------------------------------------------------------------------------------

    EndProcess();
    return true;
}
