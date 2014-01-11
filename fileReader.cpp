#include "fileReader.h"

#include <iostream>
#include <fstream>

using namespace std;

fileReader::fileReader(const std::string &filename)
{
	ifstream file(filename.c_str());
	
	if(!file.is_open()) {
		std::cerr<<"ERROR: Could not load "<<filename<<"."<<std::endl;
		return;
	}

	string str_line;

	while(getline(file, str_line, '\n')) {
		//str_line.replace(str_line.begin(),str_line.begin()+1,"");

		//std::cout<<str_line<<std::endl;
		stringstream line(str_line);
	
		string str_interpretation;
		string str_imConseqOp;
		getline(line, str_interpretation, ']');
		getline(line, str_imConseqOp, ']');

		str_interpretation.replace(str_interpretation.begin(), str_interpretation.begin()+1, "");
		str_imConseqOp.replace(str_imConseqOp.begin(), str_imConseqOp.begin()+2, "");
	
		//std::cout<<str_interpretation<<std::endl;
		//std::cout<<str_imConseqOp<<std::endl;

		line.str(str_interpretation);
		process_interpretation(line);
		stringstream imline(str_imConseqOp);
		process_imConseqOp(imline);
	}

	file.close();
}

fileReader::~fileReader()
{

}

void fileReader::process_interpretation(std::stringstream &line)
{
	std::vector<float> vI;
	//std::cout<<"I    :";
	string str_entry;
	while(getline(line, str_entry, ',')) {
		//std::cout<<str_entry;
		stringstream entry_stream(str_entry);

		float var;
		entry_stream >> var;

		//std::cout<<var;

		vI.push_back(var);
	}

	//std::cout<<std::endl;
	interpretation.push_back(vI);
}

void fileReader::process_imConseqOp(std::stringstream &line)
{
	std::vector<float> vTp;
	//std::cout<<"TP(I):";
	string str_entry;
	while(getline(line, str_entry, ',')) {
		//std::cout<<str_entry;
		stringstream entry_stream(str_entry);

		float var;
		entry_stream >> var;

		//std::cout<<var;

		vTp.push_back(var);
	}

	//std::cout<<std::endl;
	imConseqOp.push_back(vTp);
}

const vector<vector<float> > &fileReader::get_interpretation() {
	return interpretation;
}

const vector<vector<float> > &fileReader::get_imConseqOp() {
	return imConseqOp;
}
