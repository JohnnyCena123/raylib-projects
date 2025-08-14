#pragma once
#include <filesystem>
#include <functional>
#include <map>
#include <raylib.h>
namespace fs = std::filesystem;
#define LOAD_CALLBACK(type) \
	using Load##type##Callback = std::function<void(type&)>; \
	void dummy##type##Manipulator(type&)
using LoadImageCallback = std::function<void(Image&)>;
void dummyImageManipulator(Image&);
using LoadSoundCallback = std::function<void(Sound&)>;
void dummySoundManipulator(Sound&);
using LoadMusicCallback = std::function<void(Music&)>;
void dummyMusicManipulator(Music&);
class Game;
class ResourceManager {
public:
	ResourceManager() = delete;
	ResourceManager(Game& game);
	ResourceManager(ResourceManager const&) = delete;
	ResourceManager(ResourceManager&&) = delete;
	~ResourceManager();
	void init();
	void deinit();
	bool loadImage(std::string id, fs::path relativePath, LoadImageCallback manipulator = dummyImageManipulator);
	bool loadTexture(std::string id, fs::path relativePath, LoadImageCallback manipulator = dummyImageManipulator);
	bool loadSound(std::string id, fs::path relativePath, LoadSoundCallback manipulator = dummySoundManipulator);
	bool loadMusic(std::string id, fs::path relativePath, LoadMusicCallback manipulator = dummyMusicManipulator);
	Image const& getImage(std::string id) const;
	Texture2D const& getTexture(std::string id) const;
	Sound const& getSound(std::string id) const;
	Music const& getMusic(std::string id) const;
	Image const& getDummyImage() const;
	Texture2D const& getDummyTexture() const;
	Sound const& getDummySound() const;
	Music const& getDummyMusic() const;
private:
	Game& m_game;
	bool m_initialized;
	std::map<std::string, Image> m_images;
	std::map<std::string, Texture2D> m_textures;
	std::map<std::string, Sound> m_sounds;
	std::map<std::string, Music> m_musicStreams;
	Image m_dummyImage;
	Texture2D m_dummyTexture;
	Sound m_dummySound;
	Music m_dummyMusic;
};
