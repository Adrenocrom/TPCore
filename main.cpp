#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <time.h>
#include <bitset>

#include "fileReader.h"
#include "feedforward.h"
#include "core.h"

using namespace std;

int main(int argc, char** argv)
{
	int    iBasics = 0;											// number of additional units for the hidden layer
   int    iNumSteps = 10000;									// maximal number of steps
	float  fPer = 100.0f;										// percentage of trainingsdata
	int    iNumExamples = 100;									// Number of examples used in training
	float  fLearningRate = 0.1f;								// learning rate 
	float  fAlpha = 0.1f;										// momentum term
	string str_traingingset = "trainingset-10.txt";
	bool   bPrepar = true;										// filter mask in used

	int iNumProgs = 0;
	vector<string> str_programnames;
	vector<_program> vPrograms;

	string str_temp1 = string(argv[1]);
	string str_temp2 = string(argv[2]);
	string str_temp3 = string(argv[3]);
	string str_temp4 = string(argv[4]);
	string str_temp5 = string(argv[5]);
	string str_temp6 = string(argv[6]);
	string str_temp7 = string(argv[7]);
	string str_temp8 = string(argv[8]);
	string str_temp9 = string(argv[9]);

	str_traingingset = str_temp2;
	istringstream(str_temp3) >> iBasics;
	istringstream(str_temp4) >> iNumSteps;
	istringstream(str_temp5) >> fPer;
	istringstream(str_temp6) >> fLearningRate;
	istringstream(str_temp7) >> fAlpha;
	istringstream(str_temp8) >> bPrepar;
	istringstream(str_temp9) >> iNumProgs;

	for(int i = 10; i < iNumProgs+10; ++i) {
		str_programnames.push_back(string(argv[i]));		
	}

	printf("read test data ... \n");
	fileReader* pFileReader = 0;
	pFileReader = new fileReader(str_traingingset);
	int iNumIn = pFileReader->get_interpretation()[0].size();

	iNumExamples = pFileReader->get_interpretation().size();
	float fOnePer = (float)(iNumExamples)/100.0f;
	iNumExamples  = (int)(fOnePer * fPer);

	CORE* pCore = 0;
	pCore = new CORE;
	
	printf("read programs ... \n");
	for(int i = 0; i < str_programnames.size(); ++i) {
		vPrograms.push_back(pCore->readProgramFromFile(str_programnames[i]));
		printf("%d rules in the program %d\n", (int)vPrograms[i].size(), i);
	}

	
	
	printf("construct core ... \n");
	pCore->constructCore(iNumIn, iNumIn, &vPrograms[0], iBasics);
		
	pCore->trainCore(pFileReader->get_interpretation(),
						  pFileReader->get_imConseqOp(),
						  iNumSteps,
						  fLearningRate,
						  fAlpha,
						  iNumExamples,
						  bPrepar);

	if(vPrograms.size() > 1)
		for(int i = 1; i < vPrograms.size(); ++i) {
				printf("add program\n");
				pCore->addProgramToCore(&vPrograms[i]);

				pCore->trainCore(pFileReader->get_interpretation(),
						  			  pFileReader->get_imConseqOp(),
						  			  iNumSteps,
						  			  fLearningRate,
						  			  fAlpha,
						  			  iNumExamples,
						  			  bPrepar);
		}

	pCore->writeProgramToFile("resultProgram.txt", pCore->extractProgram());						  

	if(pFileReader) {
		delete pFileReader;
		pFileReader = 0;
	}

	if(pCore) {
		delete pCore;
		pCore = 0;
	}
}
