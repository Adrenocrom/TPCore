#ifndef FILEREADER_H
#define FILEREADER_H

#pragma once

#include <vector>
#include <string>
#include <sstream>

class fileReader
{
private:

	std::vector<std::vector<float> > interpretation;
	std::vector<std::vector<float> > imConseqOp;

	void process_interpretation(std::stringstream &line);
	void process_imConseqOp(std::stringstream &line);

public:

	fileReader(const std::string &filename);
	~fileReader();

	const std::vector<std::vector<float> > &get_interpretation();
	const std::vector<std::vector<float> > &get_imConseqOp();
};

#endif
