#include <raylib.h>
#include "resource-manager.hpp"
#include "game.hpp"

// TODO: find a way to avoid creating boilerplate in here
// potentially the only solution will be macros, ill see

void dummyImageManipulator(Image& image) { }
void dummySoundManipulator(Sound& image) { }
void dummyMusicManipulator(Music& image) { }

static inline bool isValidResourceDirPath(fs::path dir) {
	fs::path resourceDir = dir/"resources";
	return DirectoryExists(resourceDir.string().c_str()) &&
		FileExists((resourceDir/"icon.png").string().c_str());
};

fs::path ResourceManager::getResourceDir(bool portable) {
#ifdef PLATFORM_WEB
	fs::path ret = ".";
#else
	fs::path exeDir = GetApplicationDirectory();
	fs::path ret = exeDir;
	if (!portable) {
	#ifdef __linux__
		bool found = false;
		while (ret.has_parent_path()) {
			std::array subdirOptions{
				fs::path{"."},
				fs::path{"share"},
				fs::path{"share"}/PROJECT_NAME,
				fs::path{"usr"}/"share"/PROJECT_NAME
			};
			for (fs::path option : subdirOptions) {
				fs::path fullpath = ret/option;
				TraceLog(LOG_TRACE, "Testing %s", fullpath.string().c_str());
				if (isValidResourceDirPath(fullpath)) {
					TraceLog(LOG_INFO, "Detected %s as the parent for the resources directory", fullpath.string().c_str());
					found = true;
					break;
				}
			}
			if (found) break;
			ret = ret.parent_path();
		}
	#endif
	}
	if (!isValidResourceDirPath(ret)) {
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
			ret.string() + " is not a valid parent directory for the resources."
			).c_str(), "ok", "error", 0
		));
		TraceLog(LOG_ERROR, "Failed to locate resource dir; %s is not a valid parent directory.", ret.string().c_str());
	}
#endif
		return ret/"resources";
}

ResourceManager::ResourceManager(bool portable) : m_initialized(false) { }
ResourceManager::~ResourceManager() { }

void ResourceManager::init() {
	if (!m_initialized) {
		s_dummy<Image> = GenImageColor(1, 1, BLANK);
		s_dummy<Texture> = LoadTextureFromImage(s_dummy<Image>);

		load<Sound>("dummy", "dummy.wav");
		load<Music>("dummy", "dummy-stream.wav");
		s_dummy<Sound> = get<Sound>("dummy");
		s_dummy<Music> = get<Music>("dummy");
		s_resources<Sound>.erase("dummy");
		s_resources<Music>.erase("dummy");
		m_initialized = true;
	}
}

void ResourceManager::deinit() {
	if (m_initialized) {
		for (auto const& [id, image]   : s_resources<Image>)       unloader<Image>(image);
		for (auto const& [id, texture] : s_resources<Texture2D>)   unloader<Texture2D>(texture);
		for (auto const& [id, wave]    : s_resources<Wave>)        unloader<Wave>(wave);
		for (auto const& [id, sound]   : s_resources<Sound>)       unloader<Sound>(sound);
		for (auto const& [id, music]   : s_resources<Sound>)       unloader<Sound>(music);
		unloader<Image>(s_dummy<Image>);
		unloader<Texture2D>(s_dummy<Texture2D>);
		unloader<Wave>(s_dummy<Wave>);
		unloader<Sound>(s_dummy<Sound>);
		unloader<Music>(s_dummy<Music>);
		m_initialized = false;
	}
}
