#include "Player.hpp"
#include <whb/log.h>
#include <whb/log_udp.h>

Player::Player(const std::string& videoPath)
    : videoPath(videoPath), formatContext(nullptr), codecContext(nullptr),
      codec(nullptr), frame(nullptr), frameRGB(nullptr), swsContext(nullptr),
      packet(nullptr), window(nullptr), renderer(nullptr), texture(nullptr),
      buffer(nullptr), videoStreamIndex(-1), running(false) {}

Player::~Player() {
    stop();
}

bool Player::initialize() {
    WHBLogPrint("Initializing Player...");

    // Ouvrir le fichier vidéo
    if (avformat_open_input(&formatContext, videoPath.c_str(), nullptr, nullptr) < 0) {
        WHBLogPrint("Failed to open video file.");
        return false;
    }

    // Lire les informations du fichier
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        WHBLogPrint("Failed to find stream info.");
        return false;
    }

    // Charger le codec vidéo
    if (!loadCodec()) {
        WHBLogPrint("Failed to load codec.");
        return false;
    }

    // Préparer les buffers et frames
    if (!prepareFrames()) {
        WHBLogPrint("Failed to prepare frames.");
        return false;
    }

    // Configurer l'affichage SDL
    if (!setupDisplay()) {
        WHBLogPrint("Failed to setup display.");
        return false;
    }

    running = true;
    WHBLogPrint("Player initialized successfully.");
    return true;
}

bool Player::loadCodec() {
    // Trouver le flux vidéo
    for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1) {
        WHBLogPrint("No video stream found.");
        return false;
    }

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

    WHBLogPrint("Codec loaded successfully.");
    return true;
}

bool Player::prepareFrames() {
    frame = av_frame_alloc();
    frameRGB = av_frame_alloc();
    if (!frame || !frameRGB) {
        WHBLogPrint("Failed to allocate frames.");
        return false;
    }

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 32);
    buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    if (!buffer) {
        WHBLogPrint("Failed to allocate buffer.");
        return false;
    }

    av_image_fill_arrays(frameRGB->data, frameRGB->linesize, buffer, AV_PIX_FMT_RGB24,
                         codecContext->width, codecContext->height, 1);

    swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                                codecContext->width, codecContext->height, AV_PIX_FMT_RGB24,
                                SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsContext) {
        WHBLogPrint("Failed to initialize software scaler.");
        return false;
    }

    WHBLogPrint("Frames prepared successfully.");
    return true;
}

bool Player::setupDisplay() {
    window = SDL_CreateWindow("Wii U Video Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              codecContext->width, codecContext->height, SDL_WINDOW_SHOWN);
    if (!window) {
        WHBLogPrintf("Failed to create SDL window: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        WHBLogPrintf("Failed to create SDL renderer: %s", SDL_GetError());
        return false;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
                                 codecContext->width, codecContext->height);
    if (!texture) {
        WHBLogPrintf("Failed to create SDL texture: %s", SDL_GetError());
        return false;
    }

    WHBLogPrint("Display setup successfully.");
    return true;
}

void Player::render() {
    if (!running || !frame || !frameRGB || !texture) {
        WHBLogPrint("Render called but player is not properly initialized.");
        return;
    }

    packet = av_packet_alloc();
    if (!packet) {
        WHBLogPrint("Failed to allocate packet.");
        return;
    }

    while (av_read_frame(formatContext, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            if (avcodec_send_packet(codecContext, packet) >= 0) {
                if (avcodec_receive_frame(codecContext, frame) >= 0) {
                    sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height,
                              frameRGB->data, frameRGB->linesize);

                    SDL_UpdateTexture(texture, nullptr, frameRGB->data[0], frameRGB->linesize[0]);
                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
                    SDL_RenderPresent(renderer);
                }
            }
        }
        av_packet_unref(packet);
    }
    av_packet_free(&packet);
}

void Player::stop() {
    running = false;

    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (buffer) av_free(buffer);
    if (frame) av_frame_free(&frame);
    if (frameRGB) av_frame_free(&frameRGB);
    if (codecContext) avcodec_free_context(&codecContext);
    if (formatContext) avformat_close_input(&formatContext);

    WHBLogPrint("Player resources cleaned up.");
}
