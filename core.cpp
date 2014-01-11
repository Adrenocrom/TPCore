#include "core.h"

#include <iostream>
#include <fstream>
#include <string>

CORE::CORE()
{
	m_pCore = 0;
	m_iNumRules = 0;

	m_pProgram = 0;
	m_pProgram = new _program;

	m_fW = 1.0f;
	m_fLowWeights = 0.0000001f;
}

CORE::~CORE()
{
	if(m_pCore) {
		delete m_pCore;
		m_pCore = 0;
	}

	if(m_pProgram) {
		delete m_pProgram;
		m_pProgram = 0;
	}
}

_program CORE::readProgramFromFile(const string &filename)
{
	_program prog;
	ifstream file(filename.c_str());

	if(!file.is_open()) {
		cerr<<"ERROR: Could not load "<<filename<<"."<<std::endl;
		return prog;
	}

	string str_line;

	while(getline(file, str_line, '\n')) {
		stringstream line(str_line);
			
			string str_rule;
			while(getline(line, str_rule, ',')) {
				stringstream rule(str_rule);

				prog.push_back(process_rule(rule));
			}
	}
	
	file.close();
	return prog;
}

_rule CORE::process_rule(stringstream &rule)
{
	_rule theRule;

   string str_head;
	string str_body;
	getline(rule, str_head, '<');

	_literal lhead;
	lhead = process_literal(str_head);
	theRule.push_back(lhead);

	getline(rule, str_body);
	str_body = str_body.substr(1, string::npos);
	stringstream body(str_body);
	while(getline(body, str_body, '&')) {
		_literal lbody;
		lbody = process_literal(str_body);
		theRule.push_back(lbody);
	}

	return theRule;
}

_literal CORE::process_literal(string &literal)
{
	_literal theLiteral;
	size_t pos = literal.find_first_of("v");

	if(pos == 0) {
		theLiteral.isFail = false;
		theLiteral.isNeg  = false;
	}
	else if(pos == 1) {
		if(literal[0] == '~') {
			theLiteral.isFail = true;
			theLiteral.isNeg  = false;
		}
		else if(literal[0] == '-') {
			theLiteral.isFail = false;
			theLiteral.isNeg  = true;
		}
	}
	else {
		theLiteral.isFail = true;
		theLiteral.isNeg  = true;
	}

	literal = literal.substr(pos+1, string::npos);
	istringstream(literal) >> theLiteral.iAtom;

	return theLiteral;
}

void CORE::addRule(_rule* pRule, int* r)
{
	int R = *r+m_iBasic;

	if(pRule->size() > 0) {
		// connect the output
		_literal lhead = pRule->at(0);
		if(lhead.isFail) 	m_pCore->setWeight(1, lhead.iAtom, R, -m_fW);
		else m_pCore->setWeight(1, lhead.iAtom, R, m_fW);

		int p = 0;
		if(pRule->size() > 1) {
			for(int j = 1; j < pRule->size(); ++j) {
				_literal lbody = pRule->at(j);
				
				if(lbody.isFail) {
					m_pCore->setWeight(0, R, lbody.iAtom, -m_fW);
				}	
				else {
					p++;
					m_pCore->setWeight(0, R, lbody.iAtom, m_fW);
				}
			}
		}
			
		m_pCore->setWeight(0, R, m_iNumInputNeurons, (p - 0.5f)*m_fW);
		//m_pCore->setWeight(1, lhead.iAtom, m_iNumInputNeurons, m_fW/0.5f);
	}
}

void CORE::constructCore(int iIn, int iOut, _program* pProg, int iBasic)
{
	m_iNumInputNeurons = iIn;
	m_iNumOutputNeurons = iOut;
	m_iBasic = iBasic;
	m_iNumRules = pProg->size();
	m_iNumHiddenNeurons = m_iBasic + m_iNumRules;
	char cUnits[255];
	sprintf(cUnits, "%d", m_iNumHiddenNeurons);
	m_pCore = new feedForward(iIn, iOut, string(cUnits));
//	m_pCore->setAllWeights(m_fLowWeights);

	// thresholds of outputneurons
	for(int i = 0; i < m_pCore->getLayer()[2]; ++i) {
		m_pCore->setWeight(1, i, m_iNumHiddenNeurons, 0.5);
	}

	for(int r = 0; r < m_iNumRules; ++r)
		addRule(&pProg->at(r), &r);

	m_iBasic = m_iNumHiddenNeurons;
	m_pCore->saveWeights("startNet.txt");

	m_pProgram->clear();
	addProgram(pProg);
}

void CORE::addProgramToCore(_program* pProg)
{
	m_iNumRules = pProg->size();
	m_iNumHiddenNeurons = m_iBasic + m_iNumRules;
	char cUnits[255];
	sprintf(cUnits, "%d", m_iNumHiddenNeurons);
	
	feedForward* tempCore = 0;
	tempCore = new feedForward(m_iNumInputNeurons,
										m_iNumOutputNeurons,
										string(cUnits));

	int l = 0;
	for(int i = 0; i < m_pCore->getLayer()[l+1]; ++i) {
		for(int j = 0; j < m_pCore->getLayer()[l]+1; ++j) {
			tempCore->setWeight(l, i, j, 
					m_pCore->getWeight(l, i, j));
		}
	}

	l = 1;
	// Thresholds
	for(int i = 0; i < m_pCore->getLayer()[l+1]; ++i) {
		tempCore->setWeight(l, i, tempCore->getLayer()[l],
					m_pCore->getWeight(l, i, m_pCore->getLayer()[l]));
	}
	
	for(int i = 0; i < m_pCore->getLayer()[l+1]; ++i) {
		for(int j = 0; j < m_pCore->getLayer()[l]; ++j) {
			tempCore->setWeight(l, i, j,
				m_pCore->getWeight(l, i, j));
		}
	}

	// Zeiger umsetzen
	if(m_pCore) {
		delete m_pCore;
		m_pCore = 0;
	}

	m_pCore = tempCore;
	tempCore = 0;

	// Neue Regeln hinzufügen
	for(int r = 0; r < m_iNumRules; ++r)
		addRule(&pProg->at(r), &r);

	m_iBasic = m_iNumHiddenNeurons;
	m_pCore->saveWeights("startNet.txt");

	addProgram(pProg);
}

vector<float> CORE::runCore(vector<float> vIn)
{
	vector<float> vResult;
	
	bool bTrue = true;
	int  t = 0;
	while(bTrue) {
		vector<float> vTemp = makeClean(m_pCore->calcOutput(vIn));
		if(t > 0) {
			if(isCorrect(vTemp, vResult)) {
				bTrue = true;
			}
		}
		vResult = vTemp;
		t++;
	}

	return vResult;
}

void CORE::trainCore(vector<vector<float> > pIn,
						   vector<vector<float> > pOut,
							int 	iNumSteps,
							float fLearningRate,
							float fAlpha,
							int 	iNumExamples,
							bool  bPrepar,
							bool  bRunCore)
{
	bool bLearned = true;
	int  iStep = 0;
	vector<bool> vMask;
	int  ex = 0;

	if(bPrepar) {
		updateMaskSymbols();
		for(int a = 0; a < iNumExamples; ++a) {
			bool isOk = true;

			for(int i = 0; i < m_iNumInputNeurons; ++i) {
				if((m_vSymbols[i] == 0)&&(pIn[a][i] == 1)) {
					isOk = false;
				}
			}

			if(isOk) ex++;
			vMask.push_back(isOk);
		}
	} else {
		ex = iNumExamples;
	}
	
	printf("train core with %d examples\n", ex);

	clock_t startTime;
	clock_t endTime;
	float   fTime = 0.0f;

	startTime = clock();

	do
	{
		bLearned = true;
		for(int a = 0; a < iNumExamples; ++a) {
		if(bPrepar) 
			if(!vMask[a]) continue;
		if(!bRunCore) {
			if(!isCorrect(pOut[a],
				makeClean(m_pCore->calcOutput(pIn[a])))) {
				
				m_pCore->learnNetwork(pIn[a],
											 pOut[a],
											 fLearningRate,
											 fAlpha);

				bLearned = false;
			}
		}
		else {
			if(!isCorrect(pOut[a], runCore(pIn[a]))) {
				
				m_pCore->learnNetwork(pIn[a],
											 pOut[a],
											 fLearningRate,
											 fAlpha);

				bLearned = false;
			}
		}
		}
		iStep++;
		printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%d Step", iStep);

	} while((!bLearned)&&(iStep < iNumSteps));

	endTime = clock() - startTime;
	fTime = (float) endTime/((float)CLOCKS_PER_SEC);

	printf("\ntrained and %.3f s needed\n", fTime);

	m_pCore->saveWeights("net.txt");
}

bool CORE::isCorrect(vector<float> vIn, vector<float> vOut)
{
	float fSum = 0.0f;

	for(int a = 0; a < vIn.size(); ++a) {
		fSum += pow((vIn[a] - vOut[a]), 2);
	}

	if(fSum == 0) return true;

 	return false;
}

vector<float> CORE::makeClean(vector<float> vIn)
{
	for(int i = 0; i < vIn.size(); ++i) {
		if(vIn[i] < 0.5f) vIn[i] = 0.0f; 
		else vIn[i] = 1.0f;
	}

	return vIn;
}

void CORE::printv(vector<float> vIn)
{
	cout<<"(";

	for(int i = 0; i < vIn.size(); ++i) {
		cout<<vIn[i];
	}

	cout<<")"<<endl;
}

pair<pair<int, int>, string> CORE::readHeaderFromFile(const string filename)
{
	pair<int, int> header;
	pair<pair<int, int>, string> hidden;
	FILE* pFile = 0;

	if((pFile = fopen(filename.c_str(), "r")) == 0) {
		fclose(pFile);
		pFile = 0;
		std::cerr<<"ERROR: Could not load "<<filename<<"."<<std::endl;
	}

	int iTemp1, iTemp2;
	char cBuffer[200];

	fscanf(pFile, "%d", &iTemp1);
	fscanf(pFile, "%d", &iTemp2);
	fscanf(pFile, "%s", cBuffer);

	string str_hidden(cBuffer);

	header.first = iTemp1;
	header.second = iTemp2;

	hidden.first = header;
	hidden.second = str_hidden;

	fclose(pFile);
	pFile = 0;

	return hidden;
}

float CORE::countErrors(vector<float> vIn, vector<float> vOut)
{
	float fSum = 0.0f;

	for(int a = 0; a < vIn.size(); ++a) {
		fSum += pow((vIn[a] - vOut[a]), 2);
	}

	return fSum;
}

void CORE::updateMaskSymbols()
{
	m_vSymbols.clear();
	m_vSymbols.assign(m_iNumInputNeurons, 0);

	for(int r = 0; r < m_pProgram->size(); ++r) {
		if(m_pProgram->at(r).size() > 1) {
			for(int l = 1; l < m_pProgram->at(r).size(); ++l) {
				int i = m_pProgram->at(r)[l].iAtom;

				if(m_vSymbols[i] == 0) m_vSymbols[i] = 1;
			}
		}
	}
}

void CORE::addProgram(_program* pProg)
{
	for(int r = 0; r < pProg->size(); ++r) {
		m_pProgram->push_back(pProg->at(r));
	}
}

_program CORE::extractProgram()
{
	_program prog;
	vector<float> fInSeq;
	vector<float> fOuSeq;
	int iNumDifVec = powf(2, m_iNumInputNeurons);
	int iArray[17];
	FILE* pFiles[17];

	for(int i = 0; i < 17; ++i) {
		iArray[17] = 0;
		stringstream ss;
		ss << i;
		string str_filename = string("v");
		str_filename.append(ss.str());
		str_filename.append(".txt");
		pFiles[i] = fopen(str_filename.c_str(), "w");
		fprintf(pFiles[i], "17\n");
	}

	for(int i = 0; i < m_iNumInputNeurons; ++i) {
		fInSeq.push_back(0.0f);
	}

	for(int a = iNumDifVec-1; a >= 0; --a) {
		bitset<17> input(a);

		for(int i = m_iNumInputNeurons-1; i >= 0; --i) {
			if(input[i] == 1)
				fInSeq[i] = 1.0f;
			else
				fInSeq[i] = 0.0f;
			printf("%.0f", fInSeq[i]);
		}
		printf("\n");

		fOuSeq = makeClean(m_pCore->calcOutput(fInSeq));
		for(int o = 0; o < m_iNumOutputNeurons; ++o) {
			fprintf(pFiles[o], "%.0f\n", fOuSeq[o]);
			if(fOuSeq[o] == 1) {
				_rule rul;
				_literal lhead;
				lhead.iAtom = o;
				lhead.isFail = false;
				lhead.isNeg  = false;

				iArray[o]++;

				rul.push_back(lhead);
				for(int i = 0; i < m_iNumInputNeurons; ++i) {
					_literal lbody;
					lbody.iAtom = i;

					if(fInSeq[i] == 1) {
						lbody.isFail = false;
						lbody.isNeg  = false;
					}
					else {
						lbody.isFail = true;
						lbody.isNeg  = false;
					}

					rul.push_back(lbody);
				}

				prog.push_back(rul);
			}
		}
	}

	for(int i = 0; i < 17; ++i) {
		fclose(pFiles[i]);
		pFiles[i] = 0;
	}

	return prog;
}

void CORE::writeProgramToFile(const string &filename, _program prog)
{
	FILE* pFile = 0;

	if((pFile = fopen(filename.c_str(), "w")) == 0) {
		printf("Could not write program to file!\n");
		fclose(pFile);
		pFile = 0;
		return;
	}

	for(int r  = 0; r < prog.size(); ++r) {
		_rule    rul;
		rul = prog[r];
		_literal lhead;
		lhead = rul[0];

		if(lhead.isFail) fprintf(pFile, "~");
		if(lhead.isNeg)  fprintf(pFile, "-");

		fprintf(pFile, "v%d", lhead.iAtom);

		int iSize = rul.size();
		if(iSize > 1) 
			fprintf(pFile, "<-");
		for(int l = 1; l < iSize; ++l) {
			_literal lbody;
			lbody = rul[l];

			if(l > 1) fprintf(pFile, "&");

			if(lbody.isFail) fprintf(pFile, "~");
			if(lbody.isNeg)  fprintf(pFile, "-");

			fprintf(pFile, "v%d", lbody.iAtom);
		}
		fprintf(pFile, "\n");
	}

	if(pFile) {
		fclose(pFile);
		pFile = 0;
	}
}
