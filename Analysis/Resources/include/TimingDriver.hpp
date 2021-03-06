/// \file TimingDriver.hpp
/// \brief An abstract class that will provide the base for fitting
/// \author S. V. Paulauskas
/// \date August 8, 2016

#ifndef PIXIESUITE_TIMINGDRIVER_HPP
#define PIXIESUITE_TIMINGDRIVER_HPP

#include <utility>
#include <vector>

class TimingConfiguration;

/// An abstract class that will be used to handle timing.
class TimingDriver {
public:
    ///Default Constructor
    TimingDriver() {};

    ///Default destructor
    virtual ~TimingDriver() {};

    ///This virtual function provides results other than the phase to the user. Please look at the documentation of
    /// the children to see exactly what is returned with this vector.
    ///@TODO Not sure this is the best way to do things, but it cut the number of methods necessary by a factor of 5
    /// or 6.
    ///@return A vector containing useful information calculated in addition to the phase.
    virtual std::vector<double> GetResults(void) { return results_; }

    ///This is a virtual function that actually defines how we are going to determine the phase. We have several
    /// different implementations of how we can do this but we'll overload this method in the children to provide
    /// specific implementation.
    ///@param[in] data : The vector of data that we are going to work with. This usually means a trace or waveform.
    ///@param[in] cfg : Timing configuration to use for the various drivers.
    ///@param[in] maxInfo : The information about the maximum in a pair of <position, value> NOTE : The value of the
    /// maximum for CFD based calculations should be the extrapolated maximum.
    ///@param[in] a : The baseline information in a pair<baseline, stddev>
    ///@return The phase calculated by the algorithm.
    virtual double CalculatePhase(const std::vector<unsigned int> &data,  const TimingConfiguration &cfg,
                                  const std::pair<unsigned int, double> &max,
                                  const std::pair<double, double> baseline) { return 0.0; }

    ///@Brief Overload of the Calculate phase method to allow for data vectors of type double. We do this since we
    // cannot template a virtual method.
    virtual double CalculatePhase(const std::vector<double> &data, const TimingConfiguration &cfg,
                                  const std::pair<unsigned int, double> &max,
                                  const std::pair<double, double> baseline)  { return 0.0; }

    /// @return the amplitude from fits
    virtual double GetAmplitude(void) { return 0.0; }

    /// @return the chi^2 from the GSL fit
    virtual double GetChiSq(void) { return 0.0; }

    /// @return the chi^2dof from the GSL fit
    virtual double GetChiSqPerDof(void) { return 0.0; }
protected:
    std::vector<double> results_; //!< Vector containing results
};

#endif //PIXIESUITE_TIMINGDRIVER_HPP
