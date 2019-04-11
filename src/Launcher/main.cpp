#include <Frostwave/Engine.h>
#include <Frostwave/Graphics/VkFramework.h>

int main(int argc, char** argv)
{
	fw::Logger::Create();
	fw::Logger::SetLogLevel(fw::Logger::Level::All);
	argc; argv;

	fw::Settings settings;
	settings.graphics.vsync = true;
	settings.graphics.validation = fw::GraphicsSettings::Warning | fw::GraphicsSettings::Error;
	settings.graphics.validationType = fw::GraphicsSettings::General | fw::GraphicsSettings::Validation;

	settings.window.width = 1920;
	settings.window.height = 1080;
	settings.window.mode = fw::WindowSettings::Windowed;
	settings.window.title = "Frostwave";
	settings.window.focusFreeze = false;

	fw::Engine engine;
	engine.Init(settings);
	engine.Run();
}