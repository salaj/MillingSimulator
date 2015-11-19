#pragma once

#include "pusn_applicationBase.h"
#include "pusn_miller.h"

using namespace std;

namespace pusn{

	class PusnMenuService
	{
	public:
		Window* mainWindow;
		FileReader* reader;
		Miller* miller;
		HeightMap* heightMap;
		PusnMenuService(Window* MainWindow, FileReader* Reader, Miller* Miller, HeightMap* HeightMap)
		{
			mainWindow = MainWindow;
			reader = Reader;
			miller = Miller;
			heightMap = HeightMap;
		}
	};
}
