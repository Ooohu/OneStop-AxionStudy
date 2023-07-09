#include "TTree.h"
#include "TBranch.h"
#include "TBasket.h"
#include <stdio.h>
#include <stdlib.h>


void Add_massVar(){

	//TTree TreeName
	char file_name[] = "AxiongleeNtuples.root";

	TFile oldfile(file_name);
	TTree *oldtree;
	oldfile.GetObject("singlephotonana/vertex_tree",oldtree);

	const auto nentries = oldtree->GetEntries();
 
	TFile newfile("AxionleeNtuples_add.root","recreate");
	auto newtree = oldtree->CloneTree(0);

	double true_mass;
	double beta_cosine;
	newtree->Branch("true_mass",&true_mass,"true_mass/D");
	newtree->Branch("beta_cosine",&beta_cosine,"beta_cosine/D");


	std::vector<double>* mctruth_exiting_photon_energy = 0;
	std::vector<double>* mctruth_daughters_px = 0;
	std::vector<double>* mctruth_daughters_py = 0;
	std::vector<double>* mctruth_daughters_pz = 0;

	oldtree->SetBranchAddress("mctruth_exiting_photon_energy", &mctruth_exiting_photon_energy);
	oldtree->SetBranchAddress("mctruth_daughters_px", &mctruth_daughters_px);
	oldtree->SetBranchAddress("mctruth_daughters_py", &mctruth_daughters_py);
	oldtree->SetBranchAddress("mctruth_daughters_pz", &mctruth_daughters_pz);

	for (int i = 0; i < nentries; i ++){
		oldtree->GetEntry(i);
		true_mass = sqrt(2.0*mctruth_exiting_photon_energy->at(0)*mctruth_exiting_photon_energy->at(1)*(1.0-(mctruth_daughters_px->at(0)*mctruth_daughters_px->at(1) + mctruth_daughters_py->at(0)*mctruth_daughters_py->at(1) + mctruth_daughters_pz->at(0)*mctruth_daughters_pz->at(1))/(sqrt(mctruth_daughters_px->at(0)*mctruth_daughters_px->at(0) + mctruth_daughters_py->at(0)*mctruth_daughters_py->at(0) + mctruth_daughters_pz->at(0)*mctruth_daughters_pz->at(0))*sqrt(mctruth_daughters_px->at(1)*mctruth_daughters_px->at(1) + mctruth_daughters_py->at(1)*mctruth_daughters_py->at(1) + mctruth_daughters_pz->at(1)*mctruth_daughters_pz->at(1)))));

		beta_cosine = ((mctruth_daughters_px->at(0)+mctruth_daughters_px->at(1))*0.462372 + (mctruth_daughters_py->at(0)+mctruth_daughters_py->at(1))*0.0488541 + (mctruth_daughters_pz->at(0)+mctruth_daughters_pz->at(1))*0.885339)/sqrt(pow(mctruth_daughters_px->at(0)+mctruth_daughters_px->at(1),2)+pow(mctruth_daughters_py->at(0)+mctruth_daughters_py->at(1),2)+pow(mctruth_daughters_pz->at(0)+mctruth_daughters_pz->at(1),2));

		std::cout<<"Evaluate "<<i<<" entry with value "<<true_mass<<std::endl;
		newtree->Fill();
		std::cout<<"Copy This Entry"<<std::endl;
	}

	newtree->Print();
	newfile.Write();

}
