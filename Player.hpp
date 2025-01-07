#pragma once

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

class Player {
public:
    explicit Player(const std::string& videoPath);
    ~Player();

    bool initialize();
    void render();
    void stop();

private:
    bool loadCodec();
    bool prepareFrames();
    bool setupDisplay();

    std::string videoPath;

    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    AVCodec* codec;
    AVFrame* frame;
    AVFrame* frameRGB;
    SwsContext* swsContext;
    AVPacket* packet;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    uint8_t* buffer;
    int videoStreamIndex;

    bool running;
};
