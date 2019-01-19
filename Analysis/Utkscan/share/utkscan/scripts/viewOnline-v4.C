#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include "TSystem.h"
#include "TCanvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

void viewOnline(const std::string &filename, const int &id, const int minx, const int maxx) {
    const char *name = ("h"+std::to_string(id)).c_str();

    TFile *file = TFile::Open(filename.c_str());
    TH1D *hist1d = nullptr;
    TH2D *hist2d = nullptr;
    TH3D *hist3d = nullptr;

	file->GetObject(name, hist1d);
	if(hist1d) {
	    hist1d->GetXaxis()->SetRangeUser(minx,maxx);
	    hist1d->Draw();
	}
	else {
	    file->GetObject(name, hist2d);
	    if(hist2d) {
		    hist2d->GetXaxis()->SetRangeUser(minx,maxx);
	        hist2d->Draw("COLZ");
	    }
	    else {
	        file->GetObject(name, hist3d);
	        if(hist3d) {
			    hist3d->GetXaxis()->SetRangeUser(minx,maxx);
	            hist3d->Draw();
	        }
	        else
	            std::cout << "Couldn't figure out how to draw histogram with ID " << id << std::endl;
	    }
	}
	gSystem->ProcessEvents();	// needed to be able to access the canvas

}
