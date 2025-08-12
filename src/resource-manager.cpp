#include <raylib.h>
#include "resource-manager.hpp"
#include "game.hpp"

// TODO: find a way to avoid creating boilerplate in here
// potentially the only solution will be macros, ill see

void dummyImageManipulator(Image& image) { }
void dummySoundManipulator(Sound& image) { }
void dummyMusicManipulator(Music& image) { }

ResourceManager::ResourceManager(Game& game) : m_game(game), m_initialized(false) { }
ResourceManager::~ResourceManager() { }

void ResourceManager::init() {
	if (!m_initialized) {
		m_dummyImage = GenImageColor(1, 1, BLANK);
		m_dummyTexture = LoadTextureFromImage(m_dummyImage);
		m_images["dummy"] = m_dummyImage;
		m_textures["dummy"] = m_dummyTexture;

		loadSound("dummy", "dummy.wav");
		loadMusic("dummy", "dummy-stream.wav");
		m_dummySound = getSound("dummy");
		m_dummyMusic = getMusic("dummy");
		m_initialized = true;
	}
}

void ResourceManager::deinit() {
	if (m_initialized) {
		for (auto const& [id, image] : m_images)       { UnloadImage(image);       m_images.erase(id); }
		for (auto const& [id, texture] : m_textures) { UnloadTexture(texture);   m_textures.erase(id); }
		for (auto const& [id, sound] : m_sounds)       { UnloadSound(sound);       m_sounds.erase(id); }
		for (auto const& [id, music] : m_musicStreams) { UnloadMusicStream(music); m_musicStreams.erase(id); }
		UnloadImage(m_dummyImage);
		UnloadTexture(m_dummyTexture);
		UnloadSound(m_dummySound);
		UnloadMusicStream(m_dummyMusic);
		m_initialized = false;
	}
}

bool ResourceManager::loadImage(std::string id, fs::path relativePath, LoadImageCallback manipulator) {
	fs::path const fullPath = m_game.getResourceDir()/relativePath;

	if (!FileExists(fullPath.string().c_str())) {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Image file does not exist: %s", id.c_str(), fullPath.string().c_str());
		return false;
	}
	Image image = LoadImage(fullPath.string().c_str());
	if (!IsImageValid(image)) {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Image file failed to load: %s", id.c_str(), fullPath.string().c_str());
		return false;
	}
	if (manipulator != nullptr) manipulator(image);
	if (!IsImageValid(image)) {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Image was corrupted", id.c_str());
		UnloadImage(image);
		return false;
	}
	m_images[id] = image;
	return true;
}
bool ResourceManager::loadTexture(std::string id, fs::path relativePath, LoadImageCallback manipulator) {
	if (!loadImage(id, relativePath, manipulator)) {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Texture failed to load", id.c_str());
		return false;
	}
	m_textures[id] = LoadTextureFromImage(m_images[id]);
	UnloadImage(m_images[id]);
	m_images.erase(id);
	return true;
}
bool ResourceManager::loadSound(std::string id, fs::path relativePath, LoadSoundCallback manipulator) {
	fs::path const fullPath = m_game.getResourceDir()/relativePath;

	if (!FileExists(fullPath.string().c_str())) {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Sound file does not exist: %s", id.c_str(), fullPath.string().c_str());
		return false;
	}
	Sound sound = LoadSound(fullPath.string().c_str());
	if (!IsSoundValid(sound)) {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Sound file failed to load: %s", id.c_str(), fullPath.string().c_str());
		UnloadSound(sound);
		return false;
	}
	if (manipulator != nullptr) manipulator(sound);
	if (!IsSoundValid(sound)) {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Sound was corrupted", id.c_str());
		UnloadSound(sound);
		return true;
	}
	m_sounds[id] = sound;
	return true;
}
bool ResourceManager::loadMusic(std::string id, fs::path relativePath, LoadMusicCallback manipulator) {
	fs::path const fullPath = m_game.getResourceDir()/relativePath;

	if (!FileExists(fullPath.string().c_str())) {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Music file does not exist: %s", id.c_str(), fullPath.string().c_str());
		return false;
	}
	Music music = LoadMusicStream(fullPath.string().c_str());
	if (!IsMusicValid(music)) {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Music file failed to load: %s", id.c_str(), fullPath.string().c_str());
		UnloadMusicStream(music);
		return false;
	}
	if (manipulator != nullptr) manipulator(music);
	if (!IsMusicValid(music)) {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Music was corrupted", id.c_str());
		UnloadMusicStream(music);
		return true;
	}
	m_musicStreams[id] = music;
	return true;
}

Image const& ResourceManager::getImage(std::string id) const {
	if (m_images.contains(id)) return m_images.at(id);
	else {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Image has NOT been loaded, returning dummy", id.c_str());
		return m_dummyImage;
	}
}
Texture2D const& ResourceManager::getTexture(std::string id) const {
	if (m_textures.contains(id)) return m_textures.at(id);
	else {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Texture has NOT been loaded, returning dummy", id.c_str());
		return m_dummyTexture;
	}
}
Sound const& ResourceManager::getSound(std::string id) const {
	if (m_sounds.contains(id)) return m_sounds.at(id);
	else {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Sound has NOT been loaded, returning dummy", id.c_str());
		return m_dummySound;
	}
}
Music const& ResourceManager::getMusic(std::string id) const {
	if (m_musicStreams.contains(id)) return m_musicStreams.at(id);
	else {
		TraceLog(LOG_WARNING, "RESOURCES: ['%s'] Music has NOT been loaded, returning dummy", id.c_str());
		return m_dummyMusic;
	}
}

Image const& ResourceManager::getDummyImage() const { return m_dummyImage; }
Texture2D const& ResourceManager::getDummyTexture() const { return m_dummyTexture; }
Sound const& ResourceManager::getDummySound() const { return m_dummySound; }
Music const& ResourceManager::getDummyMusic() const { return m_dummyMusic; }
