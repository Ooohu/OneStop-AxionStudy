#include <iostream>
#include <vector>

// Read a text file and return a vector of doubles of a given column separated by a given delimiter
std::vector<double> GetNumFromText(std::string filename, int col, char delim){

	std::vector<double> vec;
	std::ifstream file(filename);
	std::string line;
	std::string val;
	int i=0;
	while(std::getline(file,line)){
		std::stringstream ss(line);
		while(std::getline(ss,val,delim)){
			if(i==col && val.find_first_not_of("0123456789.-+eE") == std::string::npos ){//need numbers
				if(val.find_first_of("-") - val.find_first_of("eE") == 1 && val.length() - val.find_first_of("-") > 3){//turn #.##e-### into 0
					val = "0";
				}
				vec.push_back(std::stod(val));
			} else if(i==col){// 0 for non-numerical values
				vec.push_back(0);
			}
			i++;
		}
		i=0;
	}
	return vec;

}

// generate random names for histograms
TString RandomName(){
	TString name = "hist";
	for(int i=0;i<10;i++){
		name += (char)(rand()%26+97);
	}
	return name;
}

TH2F* Grab2DHist( std::vector<TString> fileconf, TString cut, TString var_y_to_x, std::vector<double> binnings){

	TFile* file = new TFile(fileconf[0],"READ");
	//check if file is open
	if(!file->IsOpen()){
		std::cout << "File " << fileconf[0] << " not found." << std::endl;
		return 0;
	}
	TTree* tree = (TTree*)file->Get(fileconf[1]);
	//check if tree is open
	if(!tree){
		std::cout << "Tree " << fileconf[1] << " not found in " << fileconf[0] <<std::endl;
		return 0;
	}
	//check if binnings have 6 elements
	if(binnings.size()!=6){
		std::cout << "2DHistogram binnings must have 6 elements." << std::endl;
		return 0;
	}
//	std::cout<<"Draw "<<var_y_to_x<<" with cut "<<cut <<std::endl;

	TString tmp_name = RandomName();
	TH2F* hist = new TH2F(tmp_name, "2dhist",binnings[0],binnings[1],binnings[2],binnings[3],binnings[4],binnings[5]);

	tree->Draw(var_y_to_x+">>"+tmp_name,cut);
	//print out all the bin contents
	//CHECK
//	for(int i=0;i<hist->GetNbinsX();i++){
//		for(int j=0;j<hist->GetNbinsY();j++){
//			std::cout << hist->GetBinContent(i,j) << " ";
//		}
//		std::cout << std::endl;
//	}
	return hist;
}

void ExportPNG(TH2F* hist, TString name){
	TCanvas* c = new TCanvas("c","c",800,600);
	hist->Draw("colz");
	c->SaveAs("./output/"+name+".png");
	delete c;
}

//Normalize two 2dhistograms
void Normalize2DHist(TH2F* hist1, TH2F* hist2){
	hist1->Divide(hist2);
	hist1->Multiply(hist2);
	hist2->Divide(hist1);
	hist2->Multiply(hist1);

	hist1->Scale(1/hist1->Integral());
	hist2->Scale(1/hist2->Integral());
}


void GetAxionWeight(){
	double ma_target = 0.7; //GeV
	double ma_sigma = 0.04; //take ma_target +/- ma_sigma as the mass range

	std::string filename_Naxion = "../Inputs/ExpectedNaxions.txt";
	std::vector<TString> fileconf_axionNtuples = {"../Inputs/AxiongleeNtuples_add.root","vertex_tree"};
	std::vector<TString> fileconf_mesonNtuples = {"../Inputs/PythiaNuMIMesonProd_toy.root","PythiaTree"};
	TString fileout_finalweight = "./output/AxionWeight.root";

// Read Naxions from a text file
	std::vector<double> mas =		GetNumFromText(filename_Naxion, 0, ',');
	std::vector<double> fas =		GetNumFromText(filename_Naxion, 1, ',');
	std::vector<double> Naxions =	GetNumFromText(filename_Naxion, 2, ',');

	//CHECK Print all values of a vector
//	for(int i=0;i<mas.size();i++){
//		std::cout << mas[i] << std::endl;
//	}

// Find the closest mass to the target mass
	std::vector<double> fa_indices;//indices of the identical mass 
	for(int i=0;i<mas.size();i++){
//		std::cout<<"CHECK "<<mas[i]<< " vs "<< ma_target<<std::endl;
		if(std::abs(mas[i] - ma_target)==0){
			fa_indices.push_back(i);
		}
	}
	if(fa_indices.size()==0){
		std::cout << "Mass not found. Please correct the mass target for axions." << std::endl;
		return;
	}



//🎉Reshape Axion distribution via 2d histogram
  //Axion 2dhist with true_mass & beta_cosine	
	TString mass_cut_axion = "abs(true_mass-"+std::to_string(ma_target)+")<"+std::to_string(ma_sigma);
	TString var_y_to_x_axion = "mctruth_exiting_photon_energy[0]+mctruth_exiting_photon_energy[1]:beta_cosine";
	std::vector<double> binnings = {10,0.9999,1,8,0,15};//(xbin,xmin,xmax,ybin,ymin,ymax)

	TH2F* twodAxionHist = Grab2DHist(fileconf_axionNtuples, mass_cut_axion, var_y_to_x_axion, binnings);
	//export TH2F as an image
	ExportPNG(twodAxionHist,"Axion2DHist");

  //Meson 2dhist with E:pionDotBetaUnit = (beta_px*0.462372 + beta_py*0.0488541 + beta_pz*0.885339)/sqrt(E*E-mass*mass)
	TString var_y_to_x_meson = "E:(beta_px*0.462372 + beta_py*0.0488541 + beta_pz*0.885339)/sqrt(E*E-mass*mass)";
	TH2F* twodMesonHist = Grab2DHist(fileconf_mesonNtuples, "", var_y_to_x_meson, binnings);
	ExportPNG(twodMesonHist, "Meson2DHist");

  //Get Ratio for reweighting axion distribution
	//Find overlapped phase space of two 2dhist and normalize them
	TH2F* twodAxionHist_reweight = (TH2F*) twodAxionHist->Clone();
	TH2F* twodMesonHist_reweight = (TH2F*) twodMesonHist->Clone();
	Normalize2DHist(twodAxionHist_reweight, twodMesonHist_reweight);

	ExportPNG(twodAxionHist_reweight, "Axion2DHist_reweight1");
	ExportPNG(twodMesonHist_reweight, "Meson2DHist_reweight1");

	//Get the ratio of two 2dhist meson/axion
	TH2F* twodRatioHist = (TH2F*) twodMesonHist_reweight->Clone();
	twodRatioHist->Divide(twodAxionHist_reweight);
	ExportPNG(twodRatioHist, "Ratio2DHist_Meson2Axion");
	
//🎉Evalue the final weight for each axion events
	TFile* file_mafaaxion = TFile::Open(fileout_finalweight, "recreate");
	TTree* tree_mafaaxion = new TTree("Meson_reweight", ("Expected NAxion at " + std::to_string(ma_target)+ "GeV of 2e21 POT from NuMI").c_str());

	double falist[fa_indices.size()]; //a set of arrays
	double wgt[fa_indices.size()];

	tree_mafaaxion->Branch("fa", falist, ("fa["+std::to_string(fa_indices.size())+"]/D").c_str());
	tree_mafaaxion->Branch("wgt", wgt, ("wgt["+std::to_string(fa_indices.size())+"]/D").c_str());

	//Need some varaibles from the old tree
	TFile* file_axion = TFile::Open(fileconf_axionNtuples[0], "read");
	TTree* tree_axion = (TTree*)file_axion->Get(fileconf_axionNtuples[1]);

	double true_mass;
	double beta_cosine;
	std::vector<double>* mctruth_exiting_photon_energy = 0;

	tree_axion->SetBranchAddress("true_mass",&true_mass);
	tree_axion->SetBranchAddress("beta_cosine",&beta_cosine);
	tree_axion->SetBranchAddress("mctruth_exiting_photon_energy",&mctruth_exiting_photon_energy);

	std::cout<<"CHECK "<<__LINE__<<std::endl;
	int nEntries = tree_axion->GetEntries();

	for(int i=0; i<nEntries; i++){
		tree_axion->GetEntry(i);

		double weight_ratio = 0;
		//Check mass range
		if(std::abs(true_mass - ma_target)<= ma_sigma){//This is the mass we want, we may have a non-zero ratio to micmic mesons
			//Get the correcsponding bin weight in the twodRatioHist
			int xbin = twodRatioHist->GetXaxis()->FindBin(beta_cosine);
			int ybin = twodRatioHist->GetYaxis()->FindBin(mctruth_exiting_photon_energy->at(0)+mctruth_exiting_photon_energy->at(1));
			weight_ratio = twodRatioHist->GetBinContent(xbin,ybin);
		}

		//Fill in weights for the fa array
		for(int j=0;j<fa_indices.size();j++){
			falist[j] = fas[fa_indices[j]];
			wgt[j] = weight_ratio*Naxions[fa_indices[j]];
		}
		tree_mafaaxion->Fill();
	}

	//Write the tree
	file_mafaaxion->cd();
	tree_mafaaxion->Write();
	file_mafaaxion->Write();
	tree_mafaaxion->Print();


	}

