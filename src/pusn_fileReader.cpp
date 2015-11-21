#include "pusn_fileReader.h"

using namespace pusn;
using namespace std;

void FileReader::LoadPaths(wstring fileName)
{
	int pos = 0;
	while (fileName[pos++] != '.');
	switch (fileName[pos++])
	{
	case 'k':
		millerType = millerType::Sphere;
		break;
	case 'f':
		millerType = millerType::Cone;
		break;
	default:
		break;
	}
	millerSize = (10 * (fileName[pos++] - '0') + fileName[pos] - '0') / 2;
	ifstream input;
	//input.exceptions(ios::badbit | ios::failbit);
	input.open(fileName);
	while (!input.eof())
	{
		VertexPos vs;
		input.get();
		input.get();
		if (input.eof()) break;
		while (input.get() != 'X');
		input >> vs.Pos.x;
		input.get();
		input >> vs.Pos.y;
		input.get();
		input >> vs.Pos.z;
		paths.push_back(vs);
	}
	input.close();
}

void FileReader::Reset()
{
	pathIndex = 0;
	paths.clear();
}