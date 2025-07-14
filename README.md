# Snake

This is where I am making my second project, a Snake game!

## About this branch

### Current features

* Wrapping around the screen when you hit a border, instead of dying \
  (which is more fun, I think)
* Restarting the game when you lose
* Score and high-score (with the high-score being the best score you got within the same save file)
* Another apple is being added once every 10 points
* The snake speeds up by 1/3m/s/point
* Amazing visuals (in my opinion)
* Extensive debug window on non-Release builds using [Dear ImGui](https://github.com/ocornut/imgui)

### Planned for the future

* Main menu with settings
* Escape menu with real time configurations
* Visual customization (theme, style, etc.)
* More game-modes
* Multiplayer - potentially offline or online, either way I wouldn't say that it's coming 'soon'

## Building

The project can be built as usual like any other CMake project:

```bash
git clone https://github.com/JohnnyCena123/raylib-projects
cd raylib-projects
git checkout Snake
cmake -Bbuild -S. <optional flags - see below> -DCMAKE_BUILD_TYPE=<build type>
cmake --build build --config <build type>
```

Optional flags you can use when building:

* `-DBUILD_SHARED_LIBS=ON` - Built-into CMake, at its core just makes CMake's `add_library()` function default to \
shared (dynamic) libraries instead of static libraries when none are specified. As a bonus, in this project, it also \
renames raylib's shared library to `raylib-<build type` (e.g. `libraylib-RelWithDebInfo.so`) on non-Release builds, \
allowing everything to be in one folder.
* `-DCMAKE_LOCAL_BUILD=ON` - Enables options for building locally. Mainly helps with organizing different builds in this repo.
* `-DDONT_USE_CACHING_COMPILER=ON` - Disables looking for ccache/sccache when building.
* `-DDONT_DISABLE_WARNINGS=ON` - Does not disable warnings for dependencies.
* `-DCUSTOM_OUTPUT_OPTIONS` - Allows you to specify the Executable name, and/or build output directory; or disable them \
completely, and let CMake use the default values for them.
  * `-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=/path/to/output/directory` - Built-into CMake, \
  allows you to specify where the output Executable will be.
  * `-DEXECUTABLE_NAME=my-amazing-name` - Specifies the filename of the output Executable
* `-DSEPARATE_DIFFERENT_BUILD_TYPES_MULTI_CONFIG=ON` - Allows you to specify whether outputs of different build types \
will be placed in the same directory or not, when using a multi-config generator.
* `-DINCLUDE_TERMINAL_IN_RELEASE_WINDOWS=ON` - Allows you to include terminal when building in Release mode. Windows only.
* `-DNO_PRECOMPILED_HEADERS=ON` - Whether or not project headers will be pre-compiled before the rest of the code.
* `-DIMGUI_IN_RELEASE=ON` - Keep ImGui debug windows in Release mode
* `-DSHOW_TILE_NUMBERS=ON` - Show tile numbers on the snake
* `-DVERBOSE_LOGGING` - Logs a bunch of seemingly useless stuff. Used for debugging

### Notes

It is recommended to set the CPM_SOURCE_CACHE environment variable to a directory where dependency repositories will be cached. \
For example, raylib is relatively big (400mb) so you wouldn't want to re-clone it every time when rebuilding the project, \
or building a different project that uses [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) too.

## Contributions

I am not actively looking for contributors, however, if you find in my code:

* A bug
* Some part that is really messy/unreadable
* Any sort of bad practice I'm using

Feel free to contact me - either by opening an issue on this repository,
or messaging me on Discord (`@johnnycena123`). \
Or if you really want, you could create a pull request :)

## License

There is no license. Use my code in whatever way you want, just credit me
