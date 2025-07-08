# Snake

This is where I am making my second project, a Snake game!

## About this branch

Current features are:

* Wrapping around the screen when you hit a border, instead of dying \
  (which is more fun, I think)
* Restarting the game when you lose
* Score and high-score (with the high-score being the best score you got within the same Executable)
* Another apple is being added once every 10 points
* The game speeds up by 1 tile per second once every 2 points
* Amazing visuals (in my opinion)

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

* `-DCMAKE_LOCAL_BUILD=ON` - Enables options for building locally. Mainly helps with organizing different builds in this repo.
* `-DDONT_USE_CACHING_COMPILER=ON` - Disables looking for ccache/sccache when building.
* `-DDONT_DISABLE_WARNINGS=ON` - Does not disable warnings for dependencies.
* `-DCUSTOM_OUTPUT_OPTIONS` - Allows you to specify the Executable name, and/or build output directory; or disable them
completely, and let CMake use the default values for them.
  * `-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=...` - Built-in for CMake, allows you to specify where the output Executable will be.
  * `-DEXECUTABLE_NAME=...` - Specifies the filename of the output Executable
* `-DSEPARATE_DIFFERENT_BUILD_TYPES_MULTI_CONFIG=ON` - Allows you to specify whether outputs of different build types will be
placed in the same directory or not, when using a multi-config generator.
* `-DINCLUDE_TERMINAL_IN_RELEASE_WINDOWS=ON` - Allows you to include terminal when building in Release mode. Windows only.
* `-DNO_PRECOMPILED_HEADERS=ON` - Whether or not project headers will be pre-compiled before the rest of the code.
* `-DINCLUDE_DEBUGGING_IN_RELEASE=ON` - Keep code that's meant for debugging, in Release mode

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
