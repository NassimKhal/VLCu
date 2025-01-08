# WiiU VLC Player

## Description

**WiiU VLC Player** is a homebrew application for the Wii U console that allows you to play videos from an SD card. The project uses the following libraries:

- [FFmpeg-wiiu](https://github.com/GaryOderNichts/FFmpeg-wiiu) for video/audio decoding.
- [SDL2](https://www.libsdl.org/) for input/output management and rendering.
- [WUT](https://github.com/devkitPro/wut) for Wii U system integration.

The application is designed to work on the Wii U with the Aroma environment (version 5.5.4E).

## Project Status

This project is currently a **Work in Progress** and is not yet usable in its current state. Below is the breakdown of features already implemented and those that need to be addressed:

### Features Implemented

- Displays a list of video files on the GamePad.
- Navigation and file selection using the GamePad.
- Basic video playback on the TV (MP4 and MOV formats).
- Initial memory optimization for the Wii U.

### Features to Address

- Fix persistent Wii U loading music after application launch.
- Resolve issues with the application getting stuck on the loading screen upon exit.
- Optimize video playback to reduce stuttering and improve performance.
- Add proper handling of application exit to return cleanly to the Wii U menu.

## Prerequisites

- A Wii U with Aroma (version 5.5.4E).
- An SD card with videos placed at the root.
- DevkitPro and MSYS2 set up on your PC.

## Installation

1. Clone this repository into your development environment:

   ```bash
   git clone https://github.com/NassimKhal/WiiU_VLC_Player.git
   ```

2. Ensure the necessary libraries are installed:

   ```bash
   pacman -S wiiu-sdl2 wiiu-sdl2_mixer wiiu-sdl2_image wiiu-sdl2_ttf wiiu-physfs wiiu-mbedtls wiiu-curl wut wut-tools
   ```

3. Configure the project with the specific CMake version for the Wii U:

   ```bash
   /c/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake ./
   ```

4. Compile the project:

   ```bash
   make
   ```

5. Copy the `WiiU_VLC_Player.rpx` file into the `wiiu/apps/WiiU_VLC_Player` directory on your SD card.

## Usage

1. Insert the SD card into your Wii U and launch Aroma.
2. Select **WiiU VLC Player** from the Homebrew Launcher menu.
3. Navigate the video list using the GamePad.
4. Press `A` to play a video.

## Dependencies

- **FFmpeg-wiiu**: Video/audio decoding.
- **SDL2**: Input/output management and rendering.
- **WUT**: Wii U system integration.

## Known Issues

- The Wii U loading music sometimes persists after the application starts.
- The application may get stuck on the loading screen upon exit.
- Videos may stutter due to the hardware limitations of the Wii U.

## Contribution

Contributions are welcome! To report an issue or propose a feature, please open an [issue](https://github.com/NassimKhal/WiiU_VLC_Player/issues).

## License

This project is licensed under the [MIT](LICENSE) license.

---

**Note:** This project is still under development. Some features may not work as expected.
