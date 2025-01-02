#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>
#include <atomic>
#include <SDL2/SDL.h>

extern "C" {
    #include <SDL2/SDL.h>
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
}

#define FRAME_BUFFER_SIZE 4

class Player {
public:
    explicit Player(const std::string& filePath);
    ~Player();

    bool initialize();
    void render();
    void stop();

private:
    static int decodeFrames(void* data);

    std::string filePath;
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    const AVCodec* codec;
    struct SwsContext* swsContext;
    AVFrame* frame;
    AVPacket* packet;

    SDL_Thread* decodeThread;
    std::atomic<bool> running;

    AVFrame* frameBuffer[FRAME_BUFFER_SIZE];
    int frameBufferIndex;

    SDL_Renderer* renderer;
    SDL_Texture* texture;
};

#endif // PLAYER_HPP
