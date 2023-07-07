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



void GetAxionWeight(){

// Read Naxions from a text file
	std::vector<double> mas = GetNumFromText("../Inputs/ExpectedNaxions.txt", 0, ',');
	std::vector<double> fas = GetNumFromText("../Inputs/ExpectedNaxions.txt", 1, ',');
	std::vector<double> Naxions = GetNumFromText("../Inputs/ExpectedNaxions.txt", 2, ',');

	//Print all values of a vector
	for(int i=0;i<mas.size();i++){
		std::cout << mas[i] << std::endl;
	}

}
