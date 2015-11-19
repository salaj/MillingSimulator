#include "pusn_menu.h"

using namespace std;
using namespace pusn;


// Menu globals
Quaternion g_SpongeRotation;                 // model rotation, set by InitScene
int x_mesh = 150;
int y_mesh = 50;
int z_mesh = 150;
int precision_mesh = 100;
int animation_speed = 100;
int max_speed = 101;
bool isMaterialActive = false;
bool g_SpongeAO = true;                      // apply ambient occlusion
unsigned int g_SpongeIndicesCount = 0;       // set by BuildSponge
//Vector3 g_LightDir = { -0.5f, -0.2f, 1 };      // light direction vector
float g_CamDistance = 0.7f;                  // camera distance
//float g_BackgroundColor[] = { 0, 0, 0.5f, 1 }; // background color
bool g_Animate = true;                       // enable animation
float g_AnimationSpeed = 0.2f;               // animation speed

void TW_CALL SetPrecisionMesh(const void *value, void * a)
{
	precision_mesh = *static_cast<const int *>(value);
	container* c = static_cast<container*>(a);
	c->heightMap->materialHeight = precision_mesh;
	c->heightMap->materialWidth = precision_mesh;

	c->heightMap->Initialize();
	c->miller->Reset(c->heightMap);
}

void TW_CALL SetAnimationSpeed(const void *value, void * a)
{
	animation_speed = *static_cast<const int *>(value);
}


void TW_CALL SetXMesh(const void *value, void * a)
{
	x_mesh = *static_cast<const int *>(value);
	container* c = static_cast<container*>(a);
	c->heightMap->map_x = x_mesh;

	c->heightMap->Initialize();
	c->miller->Reset(c->heightMap);
}

void TW_CALL SetYMesh(const void *value, void * a)
{
	y_mesh = *static_cast<const int *>(value);
	container* c = static_cast<container*>(a);
	c->heightMap->map_y = y_mesh;

	c->heightMap->Initialize();
	c->miller->Reset(c->heightMap);
}

void TW_CALL SetZMesh(const void *value, void * a)
{
	z_mesh = *static_cast<const int *>(value);
	container* c = static_cast<container*>(a);
	c->heightMap->map_z = z_mesh;

	c->heightMap->Initialize();
	c->miller->Reset(c->heightMap);
}


// Callback function called by AntTweakBar to get the sponge recursion level
void TW_CALL GetXMesh(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = x_mesh;
}

void TW_CALL GetYMesh(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = y_mesh;
}

void TW_CALL GetZMesh(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = z_mesh;
}

void TW_CALL GetPrecisionMesh(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = precision_mesh;
}

void TW_CALL GetAnimationSpeed(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = animation_speed;
}


// Callback function called by AntTweakBar to enable/disable ambient occlusion
void TW_CALL SetSpongeAOCB(const void *value, void * /*clientData*/)
{
	g_SpongeAO = *static_cast<const bool *>(value);
	//BuildSponge(g_SpongeLevel, g_SpongeAO);
}


// Callback function called by AntTweakBar to get ambient occlusion state
void TW_CALL GetSpongeAOCB(void *value, void * /*clientData*/)
{
	*static_cast<bool *>(value) = g_SpongeAO;
}


void TW_CALL RunCB(void * a)
{
	container* c = static_cast<container*>(a);
	pusn::Window* mainWindow = c->mainWindow;
	//Window* mainWindow = static_cast<Window*>(a);
	OPENFILENAME ofn = { 0 };
	//TCHAR szFilters[] = TEXT("Model geometry files (*.mg1)\0*.mg1\0\0");
	TCHAR szFilePathName[_MAX_PATH] = TEXT("");
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = mainWindow->getHandle();
	//ofn.lpstrFilter = szFilters;
	ofn.lpstrFile = szFilePathName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = TEXT("Open File");
	ofn.Flags = OFN_FILEMUSTEXIST;
	GetOpenFileName(&ofn);
	wstring s = (wstring)(ofn.lpstrFile);
	if (s.size() <= 3)
		return;
	c->reader->Reset();
	c->reader->LoadPaths(s);
	c->miller->m_radius = (c->reader->millerSize) / c->heightMap->def_map_x;
	c->miller->type = c->reader->millerType;
	c->miller->Initialize();
	int breakpoint = 10;
}


void TW_CALL LoadModel(void * a)
{
	container* c = static_cast<container*>(a);
	pusn::Window* mainWindow = c->mainWindow;
	//Window* mainWindow = static_cast<Window*>(a);
	OPENFILENAME ofn = { 0 };
	//TCHAR szFilters[] = TEXT("Model geometry files (*.mg1)\0*.mg1\0\0");
	TCHAR szFilePathName[_MAX_PATH] = TEXT("");
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = mainWindow->getHandle();
	//ofn.lpstrFilter = szFilters;
	ofn.lpstrFile = szFilePathName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = TEXT("Open File");
	ofn.Flags = OFN_FILEMUSTEXIST;
	GetOpenFileName(&ofn);
	wstring s = (wstring)(ofn.lpstrFile);
	if (s.size() <= 3)
		return;

	int d = 10;
}

void TW_CALL ToogleAnimation(void * a)
{
	bool* toogle = static_cast<bool*>(a);
	if (*toogle)
		*toogle = false;
	else
		*toogle = true;
}

Menu::Menu(PusnMenuService* menuService)
{
	//ModelsManager m_modelsManager;
	//m_modelsManager.CreateModels();
	//EngineNotifier engineNotifier(&m_modelsManager);
	//m_parserManager.SetEngineNotifier(&engineNotifier);


	wstring model = L"C:\\Users\\Qba\\Documents\\Studia magisterskie\\semestr_2\\PUSN\\SymulatorFrezarki3C\\models\\JakubSala.mg1";
	string path = string(model.begin(), model.end());
	string p = "JakubSala.mg1";
	//m_parserManager.ReadModel(p);

	//int a = m_parserManager.d;
	wstring file = L"C:\\Users\\Qba\\Documents\\Studia magisterskie\\semestr_2\\PUSN\\SymulatorFrezarki3C\\paths\\t3.f12";
	menuService->reader->LoadPaths(file);
	menuService->miller->m_radius = menuService->reader->millerSize / menuService->heightMap->def_map_x;
	menuService->miller->type = menuService->reader->millerType;
	menuService->miller->SetPosition(0, 0.2, 0);
	menuService->miller->Initialize();

	/*m_miller->m_radius = 0.1f;
	m_miller->type = millerType::Sphere;
	m_miller->Initialize();*/
	animationPending = false;

	x_mesh = menuService->heightMap->map_x;
	y_mesh = menuService->heightMap->map_y;
	z_mesh = menuService->heightMap->map_z;
	precision_mesh = menuService->heightMap->materialWidth;


	// Create a tweak bar
	TwBar *bar = TwNewBar("TweakBar");
	TwDefine(" GLOBAL help='No help :)' "); // Message added to the help bar.
	int barSize[2] = { 300, 320 };
	TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, barSize);

	// Add variables to the tweak bar
	//getMainWindow()
	//container* c = new container();
	//c->mainWindow = getMainWindow();
	//c->reader = &m_reader;

	container* c = new container(
		menuService->mainWindow, //getMainWindow()
		menuService->reader,
		menuService->miller,
		menuService->heightMap);


	TwAddButton(bar, "Paths", RunCB, c, " label='Otwórz plik ze sciezkami' ");
	TwAddButton(bar, "Models", LoadModel, c, " label='Otwórz plik z modelem' ");

	TwAddVarCB(bar, "Szerokosc siatki (X)", TW_TYPE_INT32, SetXMesh, GetXMesh, c, "min=10 max=300 group='Wymiary siatki'");
	TwAddVarCB(bar, "Szerokosc siatki (Y)", TW_TYPE_INT32, SetYMesh, GetYMesh, c, "min=10 max=300 group='Wymiary siatki'");
	TwAddVarCB(bar, "Szerokosc siatki (Z)", TW_TYPE_INT32, SetZMesh, GetZMesh, c, "min=10 max=300 group='Wymiary siatki'");

	TwAddVarCB(bar, "Dokladnosc siatki", TW_TYPE_INT32, SetPrecisionMesh, GetPrecisionMesh, c, "min=10 max=180 group='Dokladnosc siatki'");

	TwAddButton(bar, "Animation", ToogleAnimation, &animationPending, " label='Start/Pauza' group='Sterowanie aplikacji'");
	TwAddVarCB(bar, "Szybkosc animacji", TW_TYPE_INT32, SetAnimationSpeed, GetAnimationSpeed, &animation_speed, "min=10 max=100 group='Sterowanie aplikacji'");
	TwAddVarRW(bar, "Wyswietlanie materialu", TW_TYPE_BOOLCPP, &isMaterialActive, "group='Sterowanie aplikacji");



	//TwAddVarCB(bar, "Ambient Occlusion", TW_TYPE_BOOLCPP, SetSpongeAOCB, GetSpongeAOCB, NULL, "group=Sponge key=o");
	//TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &g_SpongeRotation, "opened=true axisz=-z group=Sponge");
	//TwAddVarRW(bar, "Animation", TW_TYPE_BOOLCPP, &g_Animate, "group=Sponge key=a");
	//TwAddVarRW(bar, "Animation speed", TW_TYPE_FLOAT, &g_AnimationSpeed, "min=-10 max=10 step=0.1 group=Sponge keyincr=+ keydecr=-");
	//TwAddVarRW(bar, "Light direction", TW_TYPE_DIR3F, &g_LightDir, "opened=true axisz=-z showval=true");
	//TwAddVarRW(bar, "Camera distance", TW_TYPE_FLOAT, &g_CamDistance, "min=0 max=4 step=0.01 keyincr=PGUP keydecr=PGDOWN");
	//TwAddVarRW(bar, "Background", TW_TYPE_COLOR4F, &g_BackgroundColor, "colormode=hls");

}

Menu::Menu(const Menu& other)
{
}


Menu::~Menu()
{
}

bool Menu::IsMaterialActive()
{
	return isMaterialActive;
}

int Menu::GetMenuMaxSpeed()
{
	return max_speed;
}

int Menu::GetMenuAnimationSpeed()
{
	return animation_speed;
}

