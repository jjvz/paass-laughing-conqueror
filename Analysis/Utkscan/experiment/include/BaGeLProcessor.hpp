/// @file Pr270Processor.hpp
/// @brief Experiment processor for the PR270 Experiment at iThemba labs.
/// @suthor S. V. Paulauskas
/// @date January 15, 2018
/// @copyright Copyright (c) 2017 S. V. Paulauskas.
/// @copyright All rights reserved. Released under the Creative Commons Attribution-ShareAlike 4.0 International License
/// Used for feature-pr270 branch (off upstream/master)
#ifndef __BAGELPROCESSOR_HPP__
#define __BAGELPROCESSOR_HPP__
#include "EventProcessor.hpp"

#include <iostream>
#include <fstream>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH1D.h>

class TTree;

class BaGeLProcessor : public EventProcessor {
public:
    /// Default Constructor
    BaGeLProcessor();

    /// Default Destructor
    ~BaGeLProcessor();

    /// Declare the plots used in the analysis
    void DeclarePlots(void);
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
};

#endif
