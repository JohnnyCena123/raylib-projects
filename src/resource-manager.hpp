#pragma once
#include <filesystem>
#include <functional>
#include <map>
#include <raylib.h>

namespace fs = std::filesystem;

using LoadImageCallback = std::function<void(Image& image)>;
void dummyImageManipulator(Image& image);

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

	Image const& getImage(std::string id) const;
	Texture2D const& getTexture(std::string id) const;

	Image const& getEmptyImage() const;
	Texture2D const& getEmptyTexture() const;

private:

	Game& m_game;

	bool m_initialized;

	std::map<std::string, Image> m_images;
	std::map<std::string, Texture2D> m_textures;

	Image m_emptyImage;
	Texture2D m_emptyTexture;
};
