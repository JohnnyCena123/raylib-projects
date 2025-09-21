#pragma once
#include <filesystem>
#include <functional>
#include <map>
#include <raylib.h>
#include <type_traits>
namespace fs = std::filesystem;

/* some metaprogramming over here */

// we have std::is_nothrow_move_constructible but not std::is_any. i have no words
template<class T, class... Ts>
using IsAny = std::disjunction<std::is_same<T, Ts>...>;

template<class T, class... Ts>
inline bool constexpr isAny = IsAny<T, Ts...>::value;

template <class T>
inline bool constexpr isResource = isAny<T,
	Image, Texture2D,
	Wave, Sound, Music
	// no 3D resources for now
	// probably should add fonts but eh
>;
template <class T>
concept resource = isResource<T>;

template <resource T>
struct RawStruct { using Type = T; };

template <>
struct RawStruct<Texture2D> { using Type = Image; };

template <resource T>
using RawResource = typename RawStruct<T>::Type;

template <resource T>
inline bool constexpr isRawResource = std::is_same_v<T, RawResource<T>>;
template <class T>
concept rawresource = isRawResource<T>;


template <resource T>
inline char constexpr name[] = "<unknown>";

template <>
inline char constexpr name<Image>[] = "Image";
template <>
inline char constexpr name<Texture2D>[] = "Texture";
template <>
inline char constexpr name<Wave>[] = "Wave";
template <>
inline char constexpr name<Sound>[] = "Sound";
template <>
inline char constexpr name<Music>[] = "Music";

template <rawresource T>
inline std::function<T(char const*)> loader = nullptr;

template <>
inline std::function<Image(char const*)> loader<Image> = LoadImage;
template <>
inline std::function<Wave(char const*)> loader<Wave> = LoadWave;
template <>
inline std::function<Sound(char const*)> loader<Sound> = LoadSound;
template <>
inline std::function<Music(char const*)> loader<Music> = LoadMusicStream;


template <resource T>
inline std::function<T(RawResource<T>)> loaderFromRaw = nullptr;

template <>
inline std::function<Texture2D(RawResource<Texture2D>)> loaderFromRaw<Texture2D> = LoadTextureFromImage;


template <resource T>
inline std::function<bool(T)> verifier;

template <>
inline std::function<bool(Image)> verifier<Image> = IsImageValid;
template <>
inline std::function<bool(Texture2D)> verifier<Texture2D> = IsTextureValid;
template <>
inline std::function<bool(Wave)> verifier<Wave> = IsWaveValid;
template <>
inline std::function<bool(Sound)> verifier<Sound> = IsSoundValid;
template <>
inline std::function<bool(Music)> verifier<Music> = IsMusicValid;


template <resource T>
inline std::function<void(T)> unloader;

template <>
inline std::function<void(Image)> unloader<Image> = UnloadImage;
template <>
inline std::function<void(Texture2D)> unloader<Texture2D> = UnloadTexture;
template <>
inline std::function<void(Wave)> unloader<Wave> = UnloadWave;
template <>
inline std::function<void(Sound)> unloader<Sound> = UnloadSound;
template <>
inline std::function<void(Music)> unloader<Music> = UnloadMusicStream;


template <resource T>
using LoadCallback = std::function<void(RawResource<T>&)>;

template <resource T>
inline LoadCallback<T> dummyCallback = [](RawResource<T>&) { };

/* metaprogramming ends here */


static inline bool isValidResourceDirPath(fs::path dir) {
	fs::path resourceDir = dir/"resources";
	return DirectoryExists(resourceDir.string().c_str()) &&
		FileExists((resourceDir/"icon.png").string().c_str());
};

class ResourceManager {
	friend class Game;
public:

	ResourceManager() = delete;
	ResourceManager(bool portable);
	ResourceManager(ResourceManager const&) = delete;
	ResourceManager(ResourceManager&&) = delete;
	~ResourceManager();

	void init();
	void deinit();

	template <resource T>
	inline bool load(std::string id, fs::path relativePath, LoadCallback<T> manipulator = dummyCallback<T>) {
		fs::path const fullPath = s_resourceDir/relativePath;

		if constexpr (isRawResource<T>) {
			if (!FileExists(fullPath.string().c_str())) {
				TraceLog(LOG_WARNING, "RESOURCES: ['%s'] %s file does not exist: %s", id.c_str(), name<T>, fullPath.string().c_str());
				return false;
			}
			T resource = loader<T>(fullPath.string().c_str());
			if (!verifier<T>(resource)) {
				TraceLog(LOG_WARNING, "RESOURCES: ['%s'] %s file failed to load: %s", id.c_str(), name<T>, fullPath.string().c_str());
				return false;
			}
			if (manipulator != nullptr) manipulator(resource);
			if (!verifier<T>(resource)) {
				TraceLog(LOG_WARNING, "RESOURCES: ['%s'] %s resource was incorrectly manipulated", id.c_str(), name<T>);
				unloader<T>(resource);
				return false;
			}
			s_resources<T>[id] = resource;
			return true;
		} else {
			if (!load<RawResource<T>>(id, relativePath, manipulator)) {
				TraceLog(LOG_WARNING, "RESOURCES: ['%s'] raw %s failed to load for %s", id.c_str(), name<RawResource<T>>, name<T>);
				return false;
			}
			s_resources<T>[id] = loaderFromRaw<T>(s_resources<RawResource<T>>.at(id));
			unloader<RawResource<T>>(s_resources<RawResource<T>>.at(id));
			s_resources<RawResource<T>>.erase(id);
			return true;
		}
	}

	template <resource T>
	inline T get(std::string id) const {
		if (s_resources<T>.contains(id)) return s_resources<T>.at(id);
		else {
			TraceLog(LOG_WARNING, "RESOURCES: ['%s'] %s has NOT been loaded, returning dummy", id.c_str(), name<T>);
			return s_dummy<T>;
		}
	}

	template <resource T>
	inline T const& getDummy() const { return s_dummy<T>; }

private:

	static fs::path s_resourceDir;

	static bool s_initialized;

	// these 2 have to be static in order to be templated
	template <resource T>
	static std::map<std::string, T> s_resources;

	template <resource T>
	static T s_dummy;

	static fs::path getResourceDir(bool portable);
	static fs::path getDefaultResourceDir(bool portable);
	static bool verifyResourceDir(fs::path dir);

};

template <resource T>
std::map<std::string, T> ResourceManager::s_resources{};

template <resource T>
T ResourceManager::s_dummy{};
