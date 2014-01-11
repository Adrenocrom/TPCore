#include "feedforward.h"
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

feedForward::feedForward(int iNumInputNeurons,
								 int iNumOutputNeurons,
								 const string str_hiddenLayer)
{
	m_vPerLayer.push_back(iNumInputNeurons);
	createHiddenFromString(str_hiddenLayer);
	m_vPerLayer.push_back(iNumOutputNeurons);

	m_iNumLayer			    = ((int)m_vPerLayer.size())-1;
	m_iNumHiddenLayer     = m_iNumLayer-1;
	m_fLearningRate 		 = 0.07f;
	m_fAlpha					 = 0.0f;

	for(int l = 0; l < m_iNumLayer; ++l) {
		vector<float> vHiddenT;
		vector<float> vDeltT;

		for(int i = 0; i < m_vPerLayer[l+1]+1; ++i) {
			vHiddenT.push_back(-1.0f);
			vDeltT.push_back(-1.0f);
		}

		m_ppfDelt.push_back(vDeltT);
		m_ppfHidden.push_back(vHiddenT);
	}

	// alle Gewichte werden mit 1 initialisiert
	for(int l = 0; l < m_iNumLayer; ++l) {
		
		vector<vector<float> > vMatrxT2;
		vector<vector<float> > vDeltaT2;

		for(int i = 0; i < m_vPerLayer[l+1]; ++i) {

			vector<float> vMatrxT;
			vector<float> vDeltaT;
			for(int j = 0; j < m_vPerLayer[l]+1; ++j) {
				

				vMatrxT.push_back((rand() %40 + 1) / 10000000.0f);
				vDeltaT.push_back(0.2f);
			}
			vMatrxT2.push_back(vMatrxT);
			vDeltaT2.push_back(vDeltaT);
		}

		m_pppMatrix.push_back(vMatrxT2);
		m_pppDelta.push_back(vDeltaT2);
	}
}

feedForward::~feedForward()
{
}

void feedForward::setAllWeights(float fWeight)
{
	for(int l = 0; l < m_iNumLayer; ++l) {
		for(int i = 0; i < m_vPerLayer[l+1]; ++i) {
			for(int j = 0; j < m_vPerLayer[l]+1; ++j) {
				m_pppMatrix[l][i][j] = fWeight;
			}
		}
	}
}

void feedForward::setWeight(int l, int i, int j, float w)
{
	m_pppMatrix[l][i][j] = w;
}

std::vector<float> feedForward::calcOutput(std::vector<float> vInput)
{
	m_vInput = vInput;
	m_vInput.push_back(-1.0f);
	m_vOutput.clear();
	
	for(int l = 0; l < m_iNumLayer; ++l) {
		for(int i = 0; i < m_vPerLayer[l+1]; ++i) {
			// Netzinput der nächsten Schicht berrechnen
			float fNetInput = 0;

			for(int j = 0; j < m_vPerLayer[l]+1; ++j) {
				if(l == 0) 
				 fNetInput += m_pppMatrix[l][i][j] * m_vInput[j];
				else		
				 fNetInput += m_pppMatrix[l][i][j] * m_ppfHidden[l-1][j];
			}

			float fNetOutput = 0;

			fNetOutput = 1/(1+expf(-fNetInput));

			m_ppfHidden[l][i] = fNetOutput;
		}
	}

	// Output vector lesen
	for(int i = 0; i < m_vPerLayer[m_iNumLayer]; ++i) {
		m_vOutput.push_back(m_ppfHidden[m_iNumLayer-1][i]);
	}

	return m_vOutput;
}

void feedForward::learnNetwork(std::vector<float> vInput,
										 std::vector<float> vOutput,
										 float fLearningRate,
										 float fAlpha)
{
	m_fLearningRate = fLearningRate;
	m_fAlpha = fAlpha;

	// Go straight forward and calc the Output, its saved in hidden vector
	calcOutput(vInput);

	// Add Threshold


	for(int i = 0; i < m_vPerLayer[m_iNumLayer]; ++i) {
		m_ppfDelt[m_iNumLayer-1][i] = m_ppfHidden[m_iNumLayer-1][i] *
										(1 - m_ppfHidden[m_iNumLayer-1][i]) *
							(vOutput[i] - m_ppfHidden[m_iNumLayer-1][i]);
	}

	for(int l = m_iNumLayer-1; l >= 0; --l) {
		for(int j = 0; j < m_vPerLayer[l]+1; ++j) {
			float fSum = 0.0f;

			for(int i = 0; i < m_vPerLayer[l+1]; ++i) {
				fSum += m_pppMatrix[l][i][j] * m_ppfDelt[l][i];
				//printf("%d,%d,%d\n", m_pppMatrix[l+1][i][j]);
			}
			
			if(l > 0) m_ppfDelt[l-1][j] = m_ppfHidden[l-1][j] *
								         (1 - m_ppfHidden[l-1][j]) * fSum;
		}
	}

	for(int l = m_iNumLayer-1; l >= 0; --l) {
		for(int j = 0; j < m_vPerLayer[l]+1; ++j) {
			for(int i = 0; i < m_vPerLayer[l+1]; ++i) {
			   if(l > 0) {
					m_pppDelta[l][i][j] = m_fLearningRate*m_ppfDelt[l][i]*m_ppfHidden[l-1][j] +
					   						 m_fAlpha * m_pppDelta[l][i][j];			
				}
				else {
					m_pppDelta[l][i][j] = m_fLearningRate*m_ppfDelt[l][i]*m_vInput[j] +
												 m_fAlpha * m_pppDelta[l][i][j];
				}

				m_pppMatrix[l][i][j] += m_pppDelta[l][i][j];
			}
		}
	}
}

void feedForward::loadWeights(const string filename)
{
	FILE* pFile = 0;

	if((pFile = fopen(filename.c_str(), "r")) == 0) {
		fclose(pFile);
		pFile = 0;
		std::cerr<<"ERROR: Could not load "<<filename<<"."<<std::endl;
		return;
	}

	int iTemp1, iTemp2;
	char cBuffer[200];
	float fTemp;

	fscanf(pFile, "%d", &iTemp1);
	fscanf(pFile, "%d", &iTemp2);
	fscanf(pFile, "%s", cBuffer);

	for(int l = 0; l < m_iNumLayer; ++l) {
		for(int i = 0; i < m_vPerLayer[l+1]; ++i) {
			for(int j = 0; j < m_vPerLayer[l]+1; ++j) {
				fscanf(pFile, "%f", &fTemp);
				m_pppMatrix[l][i][j] = fTemp;
			}
		}
	}

	if(pFile) {
		fclose(pFile);
		pFile = 0;
	}
}

void feedForward::saveWeights(const string filename)
{
	FILE* pFile = 0;

	if((pFile = fopen(filename.c_str(), "w")) == 0) {
		fclose(pFile);
		pFile = 0;
		std::cerr<<"ERROR: Could not load "<<filename<<"."<<std::endl;
		return;
	}

	fprintf(pFile, "%d %d ", m_vPerLayer[0], m_vPerLayer[m_iNumLayer]);

	fprintf(pFile, "%d", m_vPerLayer[1]);
	for(int l = 2; l < m_iNumLayer; ++l) {
		fprintf(pFile, ",%d", m_vPerLayer[m_iNumLayer-1]);
	}

	for(int l = 0; l < m_iNumLayer; ++l) {
		for(int i = 0; i < m_vPerLayer[l+1]; ++i) {
			fprintf(pFile, "\n");
			
			for(int j = 0; j < m_vPerLayer[l]+1; ++j) {

			
				fprintf(pFile, "%f ", m_pppMatrix[l][i][j]);
			}
		}
	}

	if(pFile) {
		fclose(pFile);
		pFile = 0;
	}
}

void feedForward::createHiddenFromString(const string str_hiddenLayer)
{
	stringstream hidden(str_hiddenLayer);
	string str_hidden;
	while(getline(hidden, str_hidden, ',')) {
		int iNumHidden = 0;
		istringstream(str_hidden) >> iNumHidden;
	
		m_vPerLayer.push_back(iNumHidden);
	}
}
