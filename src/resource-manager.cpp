#include <raylib.h>
#include "resource-manager.hpp"
#include "game.hpp"

void dummyImageManipulator(Image& image) { }

ResourceManager::ResourceManager(Game& game) : m_game(game), m_initialized(false) { }
ResourceManager::~ResourceManager() { }

void ResourceManager::init() {
	if (!m_initialized) {
		m_emptyImage = GenImageColor(1, 1, BLANK);
		m_emptyTexture = LoadTextureFromImage(m_emptyImage);
		m_initialized = true;
	}
}

void ResourceManager::deinit() {
	if (m_initialized) {
		for (auto const& [id, image] : m_images) UnloadImage(image);
		for (auto const& [id, texture] : m_textures) UnloadTexture(texture);
		UnloadImage(m_emptyImage);
		UnloadTexture(m_emptyTexture);
		m_initialized = false;
	}
}

bool ResourceManager::loadImage(std::string id, fs::path relativePath, LoadImageCallback manipulator) {
	fs::path const fullPath = m_game.getrResourceDir()/relativePath;
	if (!FileExists(fullPath.string().c_str())) {
		TraceLog(LOG_WARNING, "Image file does not exist [%s]: %s", id.c_str(), fullPath.string().c_str());
		return false;
	}
	Image image = LoadImage(fullPath.string().c_str());
	if (!IsImageValid(image)) {
		TraceLog(LOG_WARNING, "Image file failed to load [%s]: %s", id.c_str(), fullPath.string().c_str());
		return false;
	}
	Image rawImage = ImageCopy(image);
	if (manipulator != nullptr) manipulator(image);
	if (!IsImageValid(image)) {
		TraceLog(LOG_INFO, "Image of id %s was incorrectly manipulated.", id.c_str());
		UnloadImage(image);
		m_images[id] = rawImage;
		return true;
	}
	UnloadImage(rawImage);
	m_images[id] = image;
	return true;
}
bool ResourceManager::loadTexture(std::string id, fs::path relativePath, LoadImageCallback manipulator) {
	if (!loadImage(id, relativePath, manipulator)) {
		TraceLog(LOG_WARNING, "Texture of id %s failed to load", id.c_str());
		return false;
	}
	m_textures[id] = LoadTextureFromImage(m_images[id]);
	UnloadImage(m_images[id]);
	m_images.erase(id);
	return true;
}

Image const& ResourceManager::getImage(std::string id) const {
	if (m_images.contains(id)) return m_images.at(id);
	else return m_emptyImage;
}
Texture2D const& ResourceManager::getTexture(std::string id) const {
	if (m_textures.contains(id)) return m_textures.at(id);
	else return m_emptyTexture;
}

Image const& ResourceManager::getEmptyImage() const { return m_emptyImage; }
Texture2D const& ResourceManager::getEmptyTexture() const { return m_emptyTexture; }
