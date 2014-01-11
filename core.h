#ifndef CORE_H
#define CORE_H

#include <math.h>

#include <string>
#include <vector>
#include <sstream>
#include <bitset>

using namespace std;

#include "feedforward.h"

#pragma once

typedef struct SLiteral
{
	int  iAtom;
	bool isFail;	// ~
	bool isNeg;
} _literal;

typedef vector<_literal> _rule;
typedef vector<_rule>	 _program;


class CORE
{
private:
	feedForward* m_pCore;
	int			 m_iBasic;
	int			 m_iNumHiddenNeurons;
	int			 m_iNumRules;
	float			 m_fW;
	float        m_fLowWeights;
	int			 m_iNumInputNeurons;
	int			 m_iNumOutputNeurons;
	vector<int>  m_vSymbols;

	_program*    m_pProgram;

	_rule    process_rule(stringstream &rule);
	_literal process_literal(string &literal);

	void addRule(_rule* pRule, int* r);
	void updateMaskSymbols();
	void addProgram(_program* pProg);

public:
	CORE();
	~CORE();

	_program readProgramFromFile(const string &filename);
	void     writeProgramToFile(const string &filename, _program prog);

	void constructCore(int iIn, int iOut, _program* pProg, int iBasic);
	void addProgramToCore(_program* pProg);
	vector<float> runCore(vector<float> vIn);
	void trainCore(vector<vector<float> > pIn,
						vector<vector<float> > pOut,
						int   iNumSteps,
						float fLearningRate,
						float fAlpha,
						int   iNumExamples,
						bool  bPrepar = true,
						bool  bRunCore = false);

	_program* getProgram() {return m_pProgram;}
	_program  extractProgram();

	void setW(float w) {m_fW = w;}
	void setLowW(float w) {m_fLowWeights = w;}

	// utils
	bool 				isCorrect(vector<float> vIn, vector<float> vOut);
	vector<float> 	makeClean(vector<float> vIn);
	void				printv(vector<float> vIn);
	pair<pair<int, int> , string> readHeaderFromFile(const string filename);
	float				countErrors(vector<float> vIn, vector<float> vOut);

};

#endif
