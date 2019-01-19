//#include "TFile.h"
//#include "TTree.h"

//int main(){
//{
//TFile *K600 = TFile::Open("sorted00337.root");
//TFile *JJ = TFile::Open("runBaGeL_337-tree.root");

//K600->cd();
{
void readK600event(){
	TFile K600("sorted00337.root");
	TTree* DATA = (TTree*)K600.Get("DATA");
	TTreeReader K600Reader("K",&K600);
	TTreeReaderValue<Int_t> evtcounter(K600Reader, "event counter");
	TTreeReaderValue<Int_t> TAC(K600Reader, "TAC");

	DATA->Draw("TAC:evtcounter>>h1(2000,0,2000,2000,0,2000)","","L");



//	while(K600Reader.Next()){
//	if (*TAC > 300 && *TAC < 310){
//		printf("TAC value: %d\n",*TAC);
//	}
	
//}

}
}
