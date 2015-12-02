#pragma once

#include <iostream>
#include <fstream>
#include <d3d11.h>
#include <d3dx10math.h>
#include "pusn_modelclass.h"
#include "pusn_vertices.h"
#include "pusn_path.h"

using namespace std;
namespace pusn{
	class FileWriter {
	public:
		wstring fileName;
		void WritePaths(string fileName, vector<Path>& paths);
		int millerSize;
		enum millerType millerType;
		//std::vector<VertexPos> paths;
		void Reset();
		int pathIndex = 0;
	};

	//enum millerType
	//{
	//	Sphere,
	//	Cone
	//};

}