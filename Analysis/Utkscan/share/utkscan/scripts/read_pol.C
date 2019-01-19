//=========Reads pol data from output file of processor
//=========  JJvZ, Dec 2016
// Usage: .x read_pol.C("out.pol"), where "out.pol" is an example of an output pol data file
// First run utkscan to populate data file "out.pol" continually
// Then, run this code which will then read the output and update display

#define DRAW_CANVAS		// Comment this line out if you do not want a canvas display
void read_pol(std::string loglable)
{
	// CONSTANTS:
	Float_t PU=0, PD=0, UNP=1;
	Int_t LU=0, LD=0, RU=0, RD=0;
	bool isUnPol;
	Double_t tmstmp;
	Int_t stm=1000;	//wait time in ms.
	static float AY = 0.99;
	static float UNPOL=1.105;
	Char_t line[128];

#ifdef DRAW_CANVAS
	Char_t text1[60], text2[60], text3[60], text4[60], text5[60], text6[60];
	TCanvas *JJC = new TCanvas("JJC","K-Line Polarisation",10,10,350,350);

	TPaveLabel* lb = new TPaveLabel(0.1,0.88,0.9,0.98,"K-Line Polarisation","T");
	lb->SetTextSize(0.99);
	lb->SetBorderSize(1);
	lb->Draw();

	TPaveLabel* lbb = new TPaveLabel(0.1,0.02,0.9,0.1,"(Click File->Quit ROOT to exit)","B");
	lbb->SetTextSize(0.99);
	lbb->SetBorderSize(0);
	lbb->Draw();

	sprintf(text5,"UNPOL = %5.2f ",UNPOL);
	TPaveLabel* lbu = new TPaveLabel(0.1,0.80,0.9,0.88,text5,"T");
	lbu->SetFillColor(0);
	lbu->SetFillStyle(0);
	lbu->SetTextSize(0.99);
	lbu->SetBorderSize(0);
	lbu->Draw();

	TPaveText *leg1=new TPaveText(0.02,0.12,0.98,0.80,"NDC");
	leg1->AddText(text5);
	sprintf(text1,"Pol #uparrow: %5.1f %%",PU);
	sprintf(text2,"Pol #downarrow: %5.1f %%",PD);
	leg1->AddText(text1);
	leg1->AddText(text2);
	leg1->SetTextSize(0.1);
	leg1->SetTextFont(42);
	leg1->SetBorderSize(1);
	leg1->SetFillColor(0);
	leg1->SetFillStyle(0);
	leg1->SetTextAlign(22);
	leg1->Draw();
#endif

    ifstream infile;
	infile.open (loglable.c_str(), ios::in);
    if (!infile) {
        std::cerr << "Uh oh, input file error! Try read(\"out.diag\"), or press ENTER to close." << std::endl;
        std::cin.get();
        exit(1);
    }
    
//	cout<<" \nReading input data from file... \n";  
    printf("\n%-10s%-10s%-9s%-9s%-9s%-9s%-9s\n", "| P-UP %  ", "| P-DWN % ", "|  TIME  ", "|   LU   ", "|   RU   ","|   LD   ","|   RD   |" ); 
	std::cout<<" ===================---------------------------------------------\n";  
   	std::string LineBuffer;
    getline(infile,LineBuffer);	// first line with titles etc.

// The reader can read very fast. If we write to disk at a rate of value/s, then reader will eventually match that rate
// (after emtying the outfile from its top) 
//---------------------------------------------------------------------------------------------------------------------	
//	loop:

	while(!gSystem->ProcessEvents()) {
		while(  infile >> isUnPol >> LU >> LD >> RU >> RD >> tmstmp ) {	// reads the time stamp in s.
			if(isUnPol) {
				UNP = RU/(LU + 1.0e-6);
			   	sprintf(line, "| UNPOL = %-9.3f | %-6.0f | %-6d | %-6d | %-6d | %-6d |",UNP,tmstmp,LU,RU,LD,RD);
			}
			else {
				PU = (LU*UNPOL - RU)/(LU*UNPOL + RU + 1.0e-6)/AY;
				PD = (RD - LD*UNPOL)/(LD*UNPOL + RD + 1.0e-6)/AY;
                                LU = (int)(LU*UNPOL);
                                LD = (int)(LD*UNPOL);

			   	sprintf(line, "| %-7.1f | %-7.1f | %-6.0f | %-6d | %-6d | %-6d | %-6d |",PU*100.,PD*100.,tmstmp,LU,RU,LD,RD);
			}
		   	std::cout<<"\r"<<line<<std::flush;	 
		   	memset(line, 0, sizeof(line));
		   	
#ifdef DRAW_CANVAS
			leg1->Clear();
			if(isUnPol) {
				sprintf(text1,"UNPOL RUN:");
				sprintf(text2,"UNPOL = %5.3f",UNP);
			}
			else { 
				sprintf(text1,"Pol #uparrow: %5.1f %%",PU*100.);
				sprintf(text2,"Pol #downarrow: %5.1f %%",PD*100.);
			}
			sprintf(text3,"L#uparrow: %6d     R#uparrow: %6d",LU,RU);
			sprintf(text4,"L#downarrow: %6d     R#downarrow: %6d",LD,RD);

			leg1->AddText(text1);
			leg1->AddText(text2);
			leg1->AddText(text3);
			leg1->AddText(text4);

			JJC->Modified();
			JJC->Update();
			gSystem->ProcessEvents();
#endif

		}  // end of file reached
		gSystem->Sleep(stm);		// waits for stm msec., then resumes to top of loop
		infile.clear(); 
	//	goto loop;
	}
	
} // end of main()
