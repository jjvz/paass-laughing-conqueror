//=========Reads pol data from output file of processor
//=========  JJvZ, Dec 2016
// Usage: .x read_unpol.C("kpol-out.dat")
//
// First run utkscan to populate data file "out.diag" continually
// Then, run this code which will then read the output and update display

// **************** Begin MAIN() *******************************************************

//#define DRAW_CANVAS		// Comment this line out if you do not want a canvas display

void read_unpol(std::string loglable)
{
	// CONSTANTS:
	Float_t PU=0, PD=0, UNP=1;
	static float UNPOL=1.0;
	Int_t LU=0, LD=0, RU=0, RD=0;
	bool isUnPol;
	Double_t tmstmp;
	Int_t no, ind=0, evts;
	Int_t stm=100;	//wait time in ms.
	Int_t maxbin=1000;
	static float AY = 0.95;
	Char_t line[128];

#ifdef DRAW_CANVAS
	TCanvas *c2 = new TCanvas("c2","Breakup geometry",700,0,600,400);
	TH1F *h01 = new TH1F("h01","K-Pol; time [s]; pol [%]",maxbin,0,maxbin);
	TH1F *h02 = new TH1F("h02","K-Pol; time [s]; pol [%]",maxbin,0,maxbin);
	h01->GetYaxis()->SetRangeUser(0,100);

 	h01->SetBarWidth(0.8);
	h01->SetBarOffset(0.1);
	h01->SetFillColor(kBlue);

	h02->SetBarWidth(0.8);
	h02->SetBarOffset(0.5);
	h02->SetFillColor(kRed);
#endif

    ifstream infile;
	infile.open (loglable.c_str(), ios::in);

    if (!infile) {
        cerr << "Uh oh, input file error! Try read(\"out.diag\"), or press ENTER to close." << endl;
        cin.get();
        exit(1);
    }

//	cout<<" \nReading input data from file... \n";  

    printf("\n%-10s%-11s%-9s%-9s%-9s%-9s\n", "| UNPOL   ", "|   TIME   ", "|   LU     ", "|   RU     ","|   LD     ","|   RD     |" ); 
	cout<<" ----------------------------------------------------------------------------\n";  
   	std::string LineBuffer;
    getline(infile,LineBuffer);	// first line with titles etc.

#ifdef DRAW_CANVAS
	h01->Draw("bar");
	h02->Draw("bar,same");
#endif

	loop:
	
// The reader can read very fast. If we write to disk at a rate of value/s, then reader will eventually match that rate
// (after emtying the outfile from its top) 
//---------------------------------------------------------------------------------------------------------------------	
	while(  infile >> isUnPol >> LU >> LD >> RU >> RD >> tmstmp) {	// reads the time stamp in s.
	  	ind++;	// counter
		UNP = LU/(RU+ 1.0e-6);

	   	sprintf(line, " UNPOL = %6.3f",UNP);
	   	sprintf(line, "| %-8.3f | %-8.0f | %-8d | %-8d | %-8d | %-8d |",UNP,tmstmp,LU,RU,LD,RD);
	   	std::cout<<"\r"<<line<<std::flush;	 
	   	memset(line, 0, sizeof(line));
	}  // end of file reached

#ifdef DRAW_CANVAS
	gStyle->SetOptStat("e");
	c2->SetLogz(0);
	c2->Modified();
	c2->Update();
	gSystem->ProcessEvents();	// needed to be able to access the canvas
#endif

	gSystem->Sleep(stm);		// waits for stm msec., then resumes to top of loop
	infile.clear(); 
	
	goto loop;
	
} // end of main()
