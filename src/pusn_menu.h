#pragma once

#include <d3d11.h>
#include "pusn_modelclass.h"
#include "pusn_miller.h"
#include "pusn_menu_service.h"
#include "pusn_window.h"

//#include "engine_Notifier.h"
//#include "parserManager.h"

using namespace std;

namespace pusn{

	struct Quaternion
	{
		float q[4];
		static Quaternion IDENTITY;
	};
	struct Vector3
	{
		float v[3];
		static Vector3 ZERO;
	};
	struct Vertex
	{
		Vector3 Position;
		Vector3 Normal;
		unsigned int AmbientColor;
	};

	class Menu
	{
	public:
		Menu(PusnMenuService*);
		Menu(const Menu&);
		~Menu();
		bool animationPending = true;

		bool IsMaterialActive();
		int GetMenuMaxSpeed();
		int GetMenuAnimationSpeed();
		//ParserManger m_parserManager;
	private:
		/*ParserManger m_parserManager;*/
	};

	class container{
	public:
		Window* mainWindow;
		FileReader* reader;
		Miller* miller;
		HeightMap* heightMap;
		container(Window* MainWindow, FileReader* Reader, Miller* Miller, HeightMap* HeightMap)
		{
			mainWindow = MainWindow;
			reader = Reader;
			miller = Miller;
			heightMap = HeightMap;
		}
	};
}