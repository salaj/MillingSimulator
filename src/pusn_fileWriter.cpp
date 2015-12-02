#include "pusn_fileWriter.h"

using namespace pusn;
using namespace std;

void FileWriter::WritePaths(string fileName, vector<Path>& paths)
{
	FILE* myWriteFile;
	myWriteFile = fopen(fileName.c_str(), "w");
	float big = 1000;
	float maximal = -big, minimal = big;
	for (int i = 0; i < paths.size(); i++)
	{
		float factor = 75;
		float loweringInMillimeters = 30;
		float offsetY = -loweringInMillimeters / factor;
		XMFLOAT3 pos = paths[i].StartPosition;
		if (pos.y < minimal)
			minimal = pos.y;
		if (pos.y > maximal)
			maximal = pos.y;
		pos.x *= factor;
		pos.y += offsetY;
		pos.y *= factor;
		pos.z *= factor;
		fprintf(myWriteFile, "N%dG01X%.3fY%.3fZ%.3f\n", i, pos.x, -pos.z, pos.y);
	}

	fclose(myWriteFile);
}

void FileWriter::Reset()
{
	pathIndex = 0;
	//paths.clear();
}