/** \file FittingAnalyzer.cpp
 * \brief Uses a chi^2 minimization to fit waveforms
 *
 * Obtains the phase of a waveform using a Chi^2 fitting algorithm
 * implemented through the GSL libraries. We have now set up two different
 * functions for this processor. One of them handles the fast SiPMT signals,
 * which tend to be more Gaussian in shape than the standard PMT signals.
 *
 * \author S. V. Paulauskas
 * \date 22 July 2011
 *
 * @TODO This currently doesn't actually set the values for the GSL fitter
 * since we have it set as a TimingDriver type. We'll have to figure out how
 * to address that.
 */
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

#include "FittingAnalyzer.hpp"
#include "GslFitter.hpp"
#include "PaassExceptions.hpp"
#include "RootFitter.hpp"

using namespace std;

FittingAnalyzer::FittingAnalyzer(const std::string &s) {
    name = "FittingAnalyzer";
    if (s == "GSL" || s == "gsl")
        driver_ = new GslFitter();
    else if (s == "ROOT" || s == "root")
        driver_ = new RootFitter();
    else {
        stringstream ss;
        ss << "FittingAnalyzer::FittingAnalyzer - The driver type \"" << s
           << "\" was unknown. Please choose a valid driver.";
        throw PaassException(ss.str());
    }
}

FittingAnalyzer::~FittingAnalyzer() {
    delete driver_;
}

void FittingAnalyzer::Analyze(Trace &trace, const ChannelConfiguration &cfg) {
    TraceAnalyzer::Analyze(trace, cfg);

    if (trace.IsSaturated() || trace.empty() || !trace.HasValidAnalysis()) {
        trace.SetPhase(0.0);
        EndAnalyze();
        return;
    }

    auto timingConfiguration = cfg.GetTimingConfiguration();

    timingConfiguration.SetQdc(trace.GetQdc());

    if (cfg.GetType() == "beta" && cfg.GetSubtype() == "double" && cfg.HasTag("timing"))
        timingConfiguration.SetIsFastSiPm(true);

    trace.SetPhase(driver_->CalculatePhase(trace.GetWaveform(), timingConfiguration, trace.GetMaxInfo(),
                                           trace.GetBaselineInfo()) + trace.GetMaxInfo().first);
    EndAnalyze();
}