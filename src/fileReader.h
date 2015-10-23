#pragma once
#include <iostream>
#include <fstream>
#include <d3d11.h>
#include <d3dx10math.h>
#include "modelclass.h"
#include "gk2_vertices.h"

using namespace std;
namespace gk2{
	class FileReader {
	public:
		wstring fileName;
		void LoadPaths(std::wstring fileName);
		int millerSize;
		enum millerType millerType;
		std::vector<VertexPos> paths;
		int pathIndex = 0;
	};

	enum millerType
	{
		Sphere,
		Cone
	};

}