/*extern "C" {
    #include <SDL2/SDL.h>
    #include <whb/log.h>
    #include <whb/log_udp.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}*/
#include "Player.hpp"
extern "C" {
    #include <libavutil/imgutils.h>
    #include <whb/log.h>
}

Player::Player(const std::string& filePath)
    : filePath(filePath), formatContext(nullptr), codecContext(nullptr), codec(nullptr),
      swsContext(nullptr), frame(nullptr), packet(nullptr), decodeThread(nullptr),
      running(false), frameBufferIndex(0), renderer(nullptr), texture(nullptr) {}

Player::~Player() {
    stop();
}

bool Player::initialize() {
    WHBLogPrint("Initializing Player...");

    // Open video file
    if (avformat_open_input(&formatContext, filePath.c_str(), nullptr, nullptr) < 0) {
        WHBLogPrintf("Failed to open video file: %s", filePath.c_str());
        return false;
    }
    WHBLogPrint("File opened successfully.");

    // Retrieve stream information
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        WHBLogPrint("Failed to retrieve stream info.");
        return false;
    }
    WHBLogPrint("Stream info retrieved successfully.");

    // Find the video stream
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        WHBLogPrint("No video stream found.");
        return false;
    }
    WHBLogPrintf("Video stream found at index %d.", videoStreamIndex);

    // Find and open codec
    codec = avcodec_find_decoder(formatContext->streams[videoStreamIndex]->codecpar->codec_id);
    if (!codec) {
        WHBLogPrint("Failed to find video decoder.");
        return false;
    }

    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        WHBLogPrint("Failed to allocate codec context.");
        return false;
    }

    if (avcodec_parameters_to_context(codecContext, formatContext->streams[videoStreamIndex]->codecpar) < 0) {
        WHBLogPrint("Failed to copy codec parameters.");
        return false;
    }

    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        WHBLogPrint("Failed to open codec.");
        return false;
    }
    WHBLogPrint("Codec opened successfully.");

    // Allocate frame and packet
    frame = av_frame_alloc();
    packet = av_packet_alloc();
    if (!frame || !packet) {
        WHBLogPrint("Failed to allocate frame or packet.");
        return false;
    }

    // Initialize software scaler
    swsContext = sws_getContext(
        codecContext->width, codecContext->height, codecContext->pix_fmt,
        codecContext->width, codecContext->height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    if (!swsContext) {
        WHBLogPrint("Failed to initialize software scaler.");
        return false;
    }

    // Initialize frame buffer
    for (int i = 0; i < FRAME_BUFFER_SIZE; i++) {
        frameBuffer[i] = av_frame_alloc();
        if (!frameBuffer[i]) {
            WHBLogPrintf("Failed to allocate frame buffer %d.", i);
            return false;
        }
    }

    WHBLogPrint("Player initialized successfully.");
    running = true;

    // Start decode thread
    decodeThread = SDL_CreateThread(decodeFrames, "DecodeThread", this);
    if (!decodeThread) {
        WHBLogPrint("Failed to create decode thread.");
        return false;
    }

    WHBLogPrint("Decode thread started successfully.");
    return true;
}

void Player::render() {
    if (frameBuffer[frameBufferIndex] && frameBuffer[frameBufferIndex]->data[0]) {
        WHBLogPrintf("Rendering frame at index %d...", frameBufferIndex);

        uint8_t* data[1] = {frameBuffer[frameBufferIndex]->data[0]};
        int linesize[1] = {frameBuffer[frameBufferIndex]->linesize[0]};
        SDL_UpdateTexture(texture, nullptr, data[0], linesize[0]);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        frameBufferIndex = (frameBufferIndex + 1) % FRAME_BUFFER_SIZE;
        WHBLogPrint("Frame rendered successfully.");
    } else {
        WHBLogPrintf("No valid frame to render at index %d.", frameBufferIndex);
    }
}

void Player::stop() {
    WHBLogPrint("Stopping Player...");
    if (running) {
        running = false;
        SDL_WaitThread(decodeThread, nullptr);
        WHBLogPrint("Decode thread stopped.");
    }

    if (swsContext) sws_freeContext(swsContext);
    if (frame) av_frame_free(&frame);
    if (packet) av_packet_free(&packet);
    if (codecContext) avcodec_free_context(&codecContext);
    if (formatContext) avformat_close_input(&formatContext);

    for (int i = 0; i < FRAME_BUFFER_SIZE; i++) {
        if (frameBuffer[i]) av_frame_free(&frameBuffer[i]);
    }
    WHBLogPrint("Player resources cleaned up.");
}

int Player::decodeFrames(void* data) {
    Player* player = static_cast<Player*>(data);

    while (player->running) {
        if (av_read_frame(player->formatContext, player->packet) >= 0) {
            if (player->packet->stream_index == player->codecContext->codec_id) {
                if (avcodec_send_packet(player->codecContext, player->packet) >= 0) {
                    WHBLogPrint("Packet sent to decoder.");
                    while (avcodec_receive_frame(player->codecContext, player->frame) >= 0) {
                        WHBLogPrint("Frame received from decoder.");
                        sws_scale(
                            player->swsContext, player->frame->data, player->frame->linesize,
                            0, player->codecContext->height,
                            player->frameBuffer[player->frameBufferIndex]->data,
                            player->frameBuffer[player->frameBufferIndex]->linesize
                        );
                        WHBLogPrintf("Frame %d decoded and stored.", player->frameBufferIndex);
                        player->frameBufferIndex = (player->frameBufferIndex + 1) % FRAME_BUFFER_SIZE;
                    }
                } else {
                    WHBLogPrint("Failed to send packet to decoder.");
                }
            }
            av_packet_unref(player->packet);
        } else {
            WHBLogPrint("Failed to read packet.");
            break;
        }
    }

    return 0;
}

