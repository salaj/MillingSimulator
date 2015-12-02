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
		FileWriter* writer;
		Miller* miller;
		HeightMap* heightMap;
		PusnMenuService(Window* MainWindow, FileReader* Reader, FileWriter* Writer, Miller* Miller, HeightMap* HeightMap)
		{
			mainWindow = MainWindow;
			reader = Reader;
			writer = Writer;
			miller = Miller;
			heightMap = HeightMap;
		}
	};
}
