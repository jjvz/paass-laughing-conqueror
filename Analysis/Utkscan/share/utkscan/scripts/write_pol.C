void write_pol(std::string polname)
{
	static int LU=0;
	static int LD=0;
	static int RU=0;
	static int RD=0;
	static bool isUnPol=false;
	static bool isFirst=true;
	Int_t stm=100;	//wait time in ms.
    Float_t E0 = 6.;
    Float_t sig = 0.3;
    Int_t kPolrunTimeSecs=0;
    
   	Float_t PU=0.86, PD=0.79;
	static float AY = 0.55;
	static float UNPOL=0.9;	// UNPOL = R/L

    bool L_gated = false;
    bool R_gated = false;

	bool isUpPolarized = false;
	bool isDwnPolarized = false;

	Float_t gr0, gr1, gr2, randval0, randvalU, randvalD;
    TRandom3 rd(0);		// rd is a member of the class TRandom3, seeded with the 0.
/*
	TCanvas *JJC = new TCanvas("JJC","JJ's test canvas",10,0,600,300);
	TCanvas *JJC2 = new TCanvas("JJC2","JJ's test canvas",10,400,600,300);
	TH1F h01("h01","Histo from a Gaussian",1000,0,10);
	TH1F h02("h02","Histo from random fill",1000,0,1);
*/    
    ofstream polfile;
	polfile.open(polname.c_str());
	
	Float_t NLU = (1+PU*AY)/2.;	// LU ---> sigL + sigR = 2*sig0
	Float_t NLD = (1-PD*AY)/2.;	// LD
	Float_t LR = 1./(1+UNPOL);	// UNPOL L/R ratio
//	Float_t ratU = (1 + 1/PU/AY)/(1/PU/AY - 1 + 1.0e-6)*UNPOL;	// LU/RU
//	Float_t ratD = (1/PD/AY - 1)/(1 + 1/PD/AY)*UNPOL;			// LD/RD
//	Float_t probLU = ratU/(ratU+1);								// LU/(LU + RU)
//	Float_t probLD = ratD/(ratD+1);								// LD/(LD + RD)
	
	loop:
//        randval0 = 1.*rd.Gaus(E0,sig);		// members of the TRandom3 class (such as rd) have inhereted functions like Rndm()
	    randvalU = 1.*rd.Rndm();		// members of the TRandom3 class (such as rd) have inhereted functions like Rndm()
	    randvalD = 1.*rd.Rndm();		// members of the TRandom3 class (such as rd) have inhereted functions like Rndm()
//		gr0 = gRandom->Gaus(E0,sig);	//gRandom->Gaus(float mean,float sigma); the width of the Gauss function is then 2*sigma, i.e. mean +- sigma.
		gr1 = gRandom->Rndm();
		gr2 = gRandom->Rndm();
/*
        h01.Fill(gr0); 
        h02.Fill(gr1);

        if(isFirst) {
        	JJC->cd();
        	h01.Draw();

        	JJC2->cd();
        	h02.Draw();
        	isFirst=false;
        }

		JJC->Modified();
		JJC->Update();         
		JJC2->Modified();
		JJC2->Update();         
*/
//		std::cout<<randval0<<"\t"<<randval1<<"\t"<<gr0<<"\t"<<gr1<<std::endl;
		if(isUnPol) {
			if(gr2 <= LR ) {
				LU++;
				LD++;
			}
			else {
				RU++;
				RD++;
			} 	
		}
		else {
			if (gr1 <= 0.5) {
				isUpPolarized=true;        
	
				if (randvalU <= NLU) {	// k-pol Left event    
				    L_gated=true;
				    LU++;        
				}
				else {	// k-pol Right event 
				    R_gated=true;
				    RU++;
				}
			}
			else {
				isDwnPolarized=true;

				if (randvalD <= NLD) {	// k-pol Left event    
				    L_gated=true;
				    LD++;        
				}
				else {	// k-pol Right event 
				    R_gated=true;
				    RD++;
				}
			}
		}
//-------------------------------------------------------------------------
//		if(isUnPol) 
		polfile << isUnPol <<"\t"<< LU <<"\t"<< LD <<"\t"<< RU <<"\t"<< RD <<"\t"<< kPolrunTimeSecs/1000. <<"\n";	// writes the time stamp in s.
//		else polfile << isUnPol <<"\t"<< (int)(LU/UNPOL) <<"\t"<< (int)(LD/UNPOL) <<"\t"<< RU <<"\t"<< RD <<"\t"<< kPolrunTimeSecs/1000. <<"\n";	// writes the time stamp in s.
		polfile << std::flush;	// flushes remaining buffer content to file
		polfile.clear();

		L_gated = false;
		R_gated = false;

		isUpPolarized = false;
		isDwnPolarized = false;
 
		gSystem->Sleep(stm);		// waits for stm msec., then resumes to top of loop
		kPolrunTimeSecs += stm;
		polfile.clear(); 
	
		goto loop;
}
