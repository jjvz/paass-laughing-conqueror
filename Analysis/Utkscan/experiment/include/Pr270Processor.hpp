/// @file Pr270Processor.hpp
/// @brief Experiment processor for the PR270 Experiment at iThemba labs.
/// @suthor S. V. Paulauskas
/// @date January 15, 2018
/// @copyright Copyright (c) 2017 S. V. Paulauskas.
/// @copyright All rights reserved. Released under the Creative Commons Attribution-ShareAlike 4.0 International License
/// Used for feature-pr270 branch (off upstream/master)
#ifndef __PR270PROCESSOR_HPP__
#define __PR270PROCESSOR_HPP__
#include "EventProcessor.hpp"

#include <iostream>
#include <fstream>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH1D.h>

class TTree;

/// Class Analyze for ISOLDE experiments 599 and 600
class Pr270Processor : public EventProcessor {
public:
    /// Default Constructor
	///Constructor taking a value as an argument
    ///@param [in] k_linepol: '1' for k_linepol measurement, '0' for A-line pol measurement */
    Pr270Processor(const unsigned int &k_linepol);

    /// Default Destructor
    ~Pr270Processor();

    /// Declare the plots used in the analysis
    void DeclarePlots(void);

	///Constructor taking a value as an argument
    ///@param [in] k_linepol: '1' for k_linepol measurement, '0' for A-line pol measurement */
//    Pr270Processor(const unsigned int &k_linepol);

  /** Preprocess the event
    * \param [in] event : the event to process
    * \return true if the preprocess was successful */
    virtual bool PreProcess(RawEvent &event);

    /// Process the event
    /// @param [in] event : the event to process
    /// @return Returns true if the processing was successful 
    bool Process(RawEvent &event);

private:
    ///Sets the detector types that are associated with this processor
    void SetAssociatedTypes(void);
    /// Method to register our tree and branches when constructing the class
    void SetupRootOutput(void);
    TTree *tree_; //!< Pointer to the tree that we're going to define in the constructor

    std::ofstream diagfile;
    std::ofstream polfile;
    unsigned int isK_linepol_;
};

#endif
