#include "pusn_butterfly.h"
#include "pusn_window.h"
#include "pusn_exceptions.h"
#include "pusn_scene.h"

#include <future>  

#include "scene.h"
#include "window.h"
#include "settings.h"
#include "exceptions.h"
#include "guiUpdater.h"

using namespace std;
using namespace pusn;

int screenWidth = 1366;
int screenHeight = 670;
int modellerWidth = screenHeight;


void runModellerFramework(gk2::Scene* scene, gk2::Window* w, Settings* settings, int cmdShow)
{
	int exitcode;
	exitcode = scene->Run(w, settings, cmdShow);
}

void initializeModellerFramework(HINSTANCE& hInstance, int cmdShow, shared_ptr<gk2::Scene>& scene,
	shared_ptr<gk2::Window>& w,
	shared_ptr<Settings>& settings,
	shared_ptr<GUIUpdater>& guiUpdater,
	shared_ptr<EngineNotifier>& engineNotifier)
{
	wstring s(L"Geometric modelling");
	w.reset(new gk2::Window(hInstance, modellerWidth, modellerWidth, &s, false));
	WindowService* windowService = w->GetWindowService();
	settings.reset(new Settings(hInstance, modellerWidth, modellerWidth, &wstring(L"Settings"), windowService->InputClass, false));
	guiUpdater.reset(new GUIUpdater(settings.get()));
	scene.reset(new gk2::Scene(hInstance, windowService->InputClass, guiUpdater.get()));
	SceneService* sceneService = scene.get()->GetSceneService();
	engineNotifier.reset(new EngineNotifier(sceneService));
	settings.get()->SetEngineNotifier(engineNotifier.get());

	scene->Initialize(w.get(), settings.get());
	w->Show(cmdShow);
	settings->Show(cmdShow);
	settings->FakeLoad();
	future<void> fut = std::async(runModellerFramework, scene.get(), w.get(), settings.get(), cmdShow);
}

void initializePusnFramework(HINSTANCE& hInstance, shared_ptr<pusn::ApplicationBase>& app, shared_ptr<pusn::Window>& win)
{
	app.reset(new pusn::Scene(hInstance));
	win.reset(new pusn::Window(hInstance, screenWidth - modellerWidth - 32, screenHeight, L"Symulator Frezarki 3D"));
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
	{
		UNREFERENCED_PARAMETER(prevInstance);
		UNREFERENCED_PARAMETER(cmdLine);


		shared_ptr<gk2::Scene> scene;
		shared_ptr<gk2::Window> w;
		shared_ptr<Settings> settings;
		shared_ptr<GUIUpdater> guiUpdater;
		shared_ptr<EngineNotifier> engineNotifier;
		int exitCode = 0;

		shared_ptr<pusn::ApplicationBase> app;
		shared_ptr<pusn::Window> win;
		try
		{
			initializePusnFramework(hInstance, app, win);
			initializeModellerFramework(hInstance, cmdShow, scene, w, settings, guiUpdater, engineNotifier);
			app->SetModelsManager(scene.get()->GetSceneService()->ModelsManager);
			exitCode = app->Run(win.get(), cmdShow);
		}
		catch (pusn::Exception& e)
		{
			MessageBoxW(NULL, e.getMessage().c_str(), L"B³¹d", MB_OK);
			exitCode = e.getExitCode();
		}
		catch (...)
		{
			MessageBoxW(NULL, L"Nieznany B³¹d", L"B³¹d", MB_OK);
			exitCode = -1;
		}
		win.reset();
		app.reset();
		return exitCode;
}