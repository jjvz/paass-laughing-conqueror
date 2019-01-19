{
TFile *midasfile = new TFile("sorted00337.root");
TTree *midastree = (TTree*)midasfile->Get("DATA");
TFile *digfile = new TFile("runBaGeL_337-tree.root");
TTree *digtree = (TTree*)digfile->Get("DATA"); 

TH1D* hEventVsRatio = new TH1D("hEventVsRatio","Event number vs Ratio between midas and digital DAQs",500,0,500);
TH2F* hEGevsX = new TH2F("hEGevsX","Dig DAQ HPGe vs X1",800,0,800,10000,0,30000);

int midasTAC;
int midasevtcounter;
double digTAC;
int digevtcounter;
double digtrigtime;
double digTACe;
double Ge_rawEnergy[48];
double X1pos;

midastree->SetBranchAddress("TAC",&midasTAC);
midastree->SetBranchAddress("evtcounter",&midasevtcounter);
midastree->SetBranchAddress("X1pos",&X1pos);
digtree->SetBranchAddress("tac_energy",&digTAC);
digtree->SetBranchAddress("Evtnum",&digevtcounter);
digtree->SetBranchAddress("trig_time",&digtrigtime);
digtree->SetBranchAddress("Ge_rawEnergy",&Ge_rawEnergy[0]);

float TACratio=0;
double tst=0;
int counter=0;
int tstevtnr=0;
double GeE;
double X1;

for(int i=0; i<300000; i++){
	midastree->GetEntry(i);
	X1 = X1pos;
	for(int j=counter; j<1000000; j++){
		digtree->GetEntry(j);
		if(digTAC>0){
			tstevtnr=digevtcounter;
			tst=digTAC;		
			GeE=Ge_rawEnergy[0];
			counter=j+1;

			TACratio = 1.*midasTAC/tst;
			std::cout<<"Event nr = " << midasevtcounter << "; TAC value = " << midasTAC << "; Digital event nr = "<< tstevtnr <<"; Digital TAC value = "<< digTAC << "; Ratio = " << TACratio << std::endl;

			//hEventVsRatio->SetBinContent(i,TACratio);
			hEGevsX ->Fill(X1,GeE);
			break;
		}
	
	}


}

hEventVsRatio->Draw();

}
