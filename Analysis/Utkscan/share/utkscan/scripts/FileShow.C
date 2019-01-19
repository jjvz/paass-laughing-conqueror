{
TFile *digfile = new TFile("runBaGeL_337-tree.root");
TTree *DATA = (TTree*)digfile->Get("DATA"); 

double tac_energy;
double Ge_rawEnergy[48];
double trig_time;
double Ge_time[48];
int Evtnum;
int ch = 0;
double cltks2tm = 1.0;     //8.0e-9*1.0e9;	// now in [ns]
bool isFirst=true;
double t0 = 0;  //156855925714.472; 	// t0 in [us]

DATA->SetBranchAddress("tac_energy",&tac_energy);
DATA->SetBranchAddress("trig_time",&trig_time);
DATA->SetBranchAddress("Ge_rawEnergy",&Ge_rawEnergy[0]);
DATA->SetBranchAddress("Ge_time",&Ge_time[0]);
DATA->SetBranchAddress("Evtnum",&Evtnum);

double tacE=0;
double geE=0;
double trigT=0, trigT0=0;
double geT=0, geT0=0;
int counter=0;
double ttm=0;
double gtm=0;
int MaxDigEntries = DATA->GetEntries();

ofstream myfile;
myfile.open ("FileShow_out.txt");

TH1D *h001 = new TH1D("h001","Gamma energies",MaxDigEntries,0,MaxDigEntries);
TH1D *h002 = new TH1D("h002","Time differences-Ge",100000,0,700000000);
TH1D *h003 = new TH1D("h003","Time differences-trig",100000,0,700000000);

	for(int j=0; j<MaxDigEntries; j++){
		DATA->GetEntry(j);
		tacE = tac_energy;		
		trigT = trig_time*cltks2tm - t0;		
		geE = Ge_rawEnergy[ch];		
		geT = Ge_time[ch]*cltks2tm - t0;		
//		if(trigT>0){
		if(trigT>0 || geE>0){
//		    t0 = geT;
		    if(trigT!=0 && isFirst) {	
                        myfile<<j<<" : First trigger :  Evt nr = "<< Evtnum << " || trig time = "<< std::setprecision (15) << trigT << std::endl;
                        isFirst=false;
                        ttm = trigT;
                    }
                    else if(!isFirst){
//			h001->SetBinContent(j,geE);
//std::cout << std::setprecision (15) << 3.14159265358979 << std::endl;
//			std::cout<<j<<" : Evt nr = "<< Evtnum << " || Ge energy["<<ch<<"] = "<<geE<<", Ge time = "<<geT<<" || trig time = "<< trigT << std::endl;
			myfile<<j<<" : Evt nr = "<< Evtnum << " || Ge energy["<<ch<<"] = "<<geE<<", Ge time = "<< std::setprecision (15) <<geT<<" us, (+ "<<geT-geT0<<" us)";
			myfile<<" || trig time = "<< std::setprecision (15) << trigT << " us, (+ "<<trigT-trigT0<<" us)"<< std::endl;
//                        ttm=trigT;
//                        gtm=geT;
//                        h002->Fill(geT-geT0);	
                        h002->Fill(geT);	
                        h003->Fill(trigT);	
		    }
            trigT0=trigT;
            geT0=geT;	
        }
	}
//	h001->Draw();
        h002->Draw();
        h003->SetLineColor(kRed);
        h003->Draw("same");

myfile.close();
}

