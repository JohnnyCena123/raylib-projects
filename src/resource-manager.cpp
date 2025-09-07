#include <raylib.h>
#ifndef PLATFORM_WEB
	#include <tinyfiledialogs.h>
#endif
#include "resource-manager.hpp"

ResourceManager::ResourceManager(bool portable) { }
ResourceManager::~ResourceManager() { }

fs::path ResourceManager::getDefaultResourceDir(bool portable) {
	fs::path ret = GetApplicationDirectory();
	(void)verifyResourceDir(ret);
	return ret/"resources";
}

bool ResourceManager::verifyResourceDir(fs::path dir) {
	if (!isValidResourceDirPath(dir)) {
		DESKTOP_ONLY(tinyfd_messageBox("Failure", (
			"Could not find the resource directory.\n"
			"Are you sure you downloaded the resources and extracted them to the right place?\n"
			"NOTE: the folder structure should look like this:\n"
			"/path/to/" PROJECT_NAME "/\n"
			"    |-- " PROJECT_NAME "\n"
			"    |-- libraries...\n"
			"    |-- resources/\n"
			"         |-- resources...\n"
			"additional info:\n" +
			dir.string() + " is not a valid parent directory for the resources."
			).c_str(), "ok", "error", 0
		));
		TraceLog(LOG_ERROR, "Failed to locate resource dir; %s is not a valid parent directory.", dir.string().c_str());
		return false;
	} else return true;
}

void ResourceManager::init() {
	if (!s_initialized) {
		s_dummy<Image> = GenImageColor(1, 1, BLANK);
		s_dummy<Texture> = LoadTextureFromImage(s_dummy<Image>);

		load<Sound>("dummy", "dummy.wav");
		load<Music>("dummy", "dummy-stream.wav");
		s_dummy<Sound> = get<Sound>("dummy");
		s_dummy<Music> = get<Music>("dummy");
		s_resources<Sound>.erase("dummy");
		s_resources<Music>.erase("dummy");
		s_initialized = true;
	}
}
void ResourceManager::deinit() {
	if (s_initialized) {
		for (auto const& [id, image]     : s_resources<Image>)       unloader<Image>(image);
		for (auto const& [id, texture] : s_resources<Texture2D>)   unloader<Texture2D>(texture);
		for (auto const& [id, wave]       : s_resources<Wave>)        unloader<Wave>(wave);
		for (auto const& [id, sound]     : s_resources<Sound>)       unloader<Sound>(sound);
		for (auto const& [id, music]     : s_resources<Sound>)       unloader<Sound>(music);
		unloader<Image>(s_dummy<Image>);
		unloader<Texture2D>(s_dummy<Texture2D>);
		unloader<Wave>(s_dummy<Wave>);
		unloader<Sound>(s_dummy<Sound>);
		unloader<Music>(s_dummy<Music>);
		s_initialized = false;
	}
}

fs::path ResourceManager::s_resourceDir{};
bool ResourceManager::s_initialized = false;
