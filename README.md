<img src="https://github.com/elnormous/ouzel/blob/master/img/ouzel.png" alt="ouzel" width=128>

# Ouzel v0.40

[![Build Status](https://api.travis-ci.org/elnormous/ouzel.svg?branch=master)](https://travis-ci.org/elnormous/ouzel) [![Build Status](https://ci.appveyor.com/api/projects/status/dp8av7iegdjs6xuj?svg=true)](https://ci.appveyor.com/project/elnormous/ouzel) [![Quality Gate](https://sonarcloud.io/api/project_badges/measure?project=Ouzel&metric=alert_status)](https://sonarcloud.io/dashboard?id=Ouzel) [![Join the chat at https://gitter.im/ouzelengine/Lobby](https://badges.gitter.im/elnormous/ouzel.svg)](https://gitter.im/elnormous/ouzel?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Ouzel is a C++ game engine mainly targeted for development of 2D games.

Supported platforms:

* Windows 7, 8, 10
* macOS 10.8+
* Linux
* iOS 8+
* tvOS 9+
* Android 3.0+
* Emscripten ([sample](http://www.ouzel.org/samples/))

Supported rendering backends:

* Direct3D 11
* OpenGL 2, OpenGL 3 and OpenGL 4
* OpenGL ES 2 and OpenGL ES 3
* Metal 

Supported audio backends:

* XAudio 2
* DirectSound
* CoreAudio
* OpenAL
* OpenSL ES
* ALSA

## Features

* Cross-platform (Windows, macOS, iOS, tvOS, Android, Linux, and Emscripten targets supported)
* Multi-threaded (separate threads for rendering, sound, and game)
* 2D and 3D scene management
* GUI helper classes and management
* Bitmap and true-type font support
* Multiple side-by-side viewport support
* XInput, DirectInput, IOKit, Apple GameController, and Linux evdev gamepad support
* Actor animation (including tweening) system
* Particle systems
* Resource caching system
* Localization support via loading string translations and UTF-8 string support
* Software audio mixer for sound effect playback
* High DPI support on Windows, macOS, and iOS
* Easy to install (just pull the repository and it's subrepositories and build it)

## Example app

The following code will open create a scene with a sprite in the center of it:

```cpp
#include "ouzel.hpp"

class Example: public ouzel::Application
{
public:
    Example():
        assets(ouzel::engine->getCache())
    {
        assets->loadAsset(ouzel::assets::Loader::IMAGE, "player.png");
        ouzel::engine->getSceneManager().setScene(&scene);
        scene.addLayer(&layer);
        cameraActor.addComponent(&camera);
        layer.addChild(&cameraActor);
        playerSprite.init("player.png");
        player.addComponent(&playerSprite);
        layer.addChild(&player);
    }

private:
    ouzel::scene::Scene scene;
    ouzel::scene::Layer layer;
    ouzel::scene::Camera camera;
    ouzel::scene::Actor cameraActor;
    ouzel::scene::Sprite playerSprite;
    ouzel::scene::Actor player;
    ouzel::assets::Bundle assets;
}

std::unique_ptr<ouzel::Application> ouzel::main(const std::vector<std::string>& args)
{
    return std::unique_ptr<ouzel::Application>(new Example());
}
```

## Showcase

2D platformer [Bearslayer](http://store.steampowered.com/app/460210) is being developed using Ouzel engine.

[![Bearslayer](https://github.com/elnormous/ouzel/blob/master/img/bearslayer.gif "Bear Slayer")](https://www.youtube.com/watch?v=q-O8-hpvJ5A)

## Compilation

GNU makefile, Xcode project, and Visual Studio project files are located in the "build" directory. Makefile and project files for sample project are located in the "samples" directory.

You will need to download OpenGL (e.g. Mesa), ALSA, and OpenAL drivers installed in order to build Ouzel on Linux. For x86 Linux also libx11, libxcursor, libxi, and libxss are required.

To build Ouzel with Emscripten, pass "platform=emscripten" to "make" command, but make sure that you have Emscripten SDK installed before doing so:

```shell
$ make platform=emscripten
```

You can build Android samples and run them on an Android device by executing the following commands in "samples/android" directory (Android SDK and NDK must be installed and added to PATH):

```shell
$ gradle assembleDebug
$ gradle installDebug
$ adb shell am start -n org.ouzel/org.ouzel.MainActivity
```

## System requirements
* Windows 7+ with Visual Studio 2015 or Visual Studio 2017
* macOS 10.10+ with Xcode 7.2+
* Any reasonable new Linux distro (x86 and ARM are supported)

## Getting help

You can ask question in the following locations:

* Ouzel Twitter account: https://twitter.com/ouzelengine
* Ouzel server on Discord: https://discord.gg/4sWuJE8
* #ouzel channel on the Freenode network ([Freenode Webchat](http://webchat.freenode.net/?channels=ouzel))
* Author of the Ouzel engine: https://twitter.com/elnormous
* Development roadmap: https://trello.com/b/5tRlUXKR/ouzel-roadmap

## License

Ouzel codebase is licensed under the BSD license. Please refer to the LICENSE file for detailed information.
