#pragma once

#include <iostream>
#include <fstream>
#include <d3d11.h>
#include <d3dx10math.h>
#include "pusn_modelclass.h"
#include "pusn_vertices.h"

using namespace std;
namespace pusn{
	class FileReader {
	public:
		wstring fileName;
		void LoadPaths(std::wstring fileName);
		int millerSize;
		enum millerType millerType;
		std::vector<VertexPos> paths;
		void Reset();
		int pathIndex = 0;
	};

	enum millerType
	{
		Sphere,
		Cone
	};

}