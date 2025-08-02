# Raylib Projects

This is a repository for the projects I make using Raylib,
all in one repository to not create too many separate repositories. \
Each branch is a separate project, but sometimes I also make a temporary branch
to test stuff and/or add stuff to another branches without flooding the commit history.

Current projects I'm working on:

* [Tic Tac Toe](https://github.com/JohnnyCena123/raylib-projects/tree/TicTacToe)
  * [Latest development build](https://github.com/JohnnyCena123/raylib-projects/releases/tag/nightly-tic-tac-toe)
  * [Built binaries](https://github.com/JohnnyCena123/raylib-projects/releases/tag/TicTacToe-v1.0.0)
* [Snake](https://github.com/JohnnyCena123/raylib-projects/tree/Snake)
  * [Latest development build](https://github.com/JohnnyCena123/raylib-projects/releases/tag/nightly-snake)
  * [Built binaries](https://github.com/JohnnyCena123/raylib-projects/releases/tag/Snake-v1.0.0)

More info about each branch can be found in its own `README.md`.

## About this branch

This branch is a template I use whenever I want to start a new project. \
E.g. I'm starting to work on a new game, so I just copy this branch and
hop right into coding.

## Installing

There are 4 options for installing the projects in this repository.

1. Downloading from the latest release of each one (currently there are such only for
  [Tic Tac Toe](https://github.com/JohnnyCena123/raylib-projects/releases/tag/TicTacToe-v1.0.0) and
  [Snake](https://github.com/JohnnyCena123/raylib-projects/releases/tag/Snake-v1.0.0))
2. Downloading from the development build of the latest commit, e.g.
  [snake](github.com/JohnnyCena123/raylib-projects/releases/tag/nightly-snake)
3. Downloading CI artifacts - practically the same as getting them from nightly releases.
4. [Building from source](#building)

### Portable mode

If you want the application to run in portable mode, you can either pass the `-DBUILD_PORTABLE_APPLICATION=ON`
flag to CMake when building, or simply add a `.portable-application` file besides the executable - in the same directory.
The folder structure should look like this:

```theres-no-language-for-this-really-so-stop-warning-me-markdown-lint
/path/to/installed/project/
├── .portable-application   -- if it was not already built with the portable CMake flag
├── <Executable>            -- the actual application
├── <libraries...>          -- shared libaries - e.g. libraylib.dll, libimgui.so, ...
└── resources/              -- resources directory
   └── <resources...>       -- resource - images, sounds, fonts, etc.
```

## Building

The project can be built as usual like any other CMake project:

```bash
git clone https://github.com/JohnnyCena123/raylib-projects
cd raylib-projects
git checkout <branch - project>
cmake -Bbuild -S. <optional flags - see below> -DCMAKE_BUILD_TYPE=<build type>
cmake --build build --config <build type>
```

Optional flags you can use when building:

* `-DBUILD_SHARED_LIBS=ON` - Built-into CMake, at its core just makes CMake's `add_library()` function default to \
shared (dynamic) libraries instead of static libraries when none are specified. As a bonus, in this project, it also \
renames raylib's shared library to `raylib-<build type` (e.g. `libraylib-RelWithDebInfo.so`) on non-Release builds, \
allowing everything to be in one folder.
* `-DUSE_CACHING_COMPILER=OFF` - Disables looking for ccache/sccache when configuring the project.
* `-DALL_BUILD_TYPES_TOGETHER=OFF` - Places the output of each build type in its own separate directory.
* `-DCMAKE_LOCAL_BUILD=ON` - Enables options for building locally. \
  Mainly helps with organizing builds of different build types or different projects.
* `-DCUSTOM_OUTPUT_OPTIONS` - Allows you to specify the Executable name, and/or build output directory; or disable them \
completely, and let CMake use the default values for them.
  * `-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=/path/to/output/directory` - Built-into CMake, \
  allows you to specify where the output Executable will be.
  * `-DBIN_SUFFIX=-foobar` - Specifies the suffix for the filename of the output Executable
* `-DDISABLE_WARNINGS=OFF` - Disables disabling warnings for dependencies.
* `-DIMGUI_IN_RELEASE=ON` - Keep ImGui debug windows in Release mode
* `-DINCLUDE_TERMINAL_IN_RELEASE=ON` - Allows you to include the terminal popup when building for Windows in Release mode.
* `-DINCLUDE_ICON=OFF` - Lets you decide whether or not the application will have a taskbar/explorer icon on Windows.
* `-DUSE_PRECOMPILED_HEADERS=OFF` - Whether or not project headers will be pre-compiled before the rest of the code.
* `-DBUILD_PORTABLE_APPLICATION=ON` - Determines whether the application should be built in portable mode, e.g. everything in 1 folder

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
Or if you really want, you could open a pull request :)

## License

There is no license. Use my code in whatever way you want, just credit me
