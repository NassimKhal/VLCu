#include "Player.hpp"
extern "C" {
    #include <whb/proc.h>
    #include <whb/log.h>
    #include <whb/log_udp.h>
    #include <coreinit/screen.h>
    #include <coreinit/memory.h>
    #include <coreinit/memdefaultheap.h>
    #include <coreinit/thread.h>
    #include <coreinit/time.h>
    #include <SDL2/SDL.h>
    #include <sysapp/launch.h>
}

int main(int argc, char** argv) {
    WHBLogUdpInit();
    WHBLogPrint("Wii U Video Player Initializing...");

    if (!WHBProcInit()) {
        WHBLogPrint("Failed to initialize WHBProc.");
        WHBLogUdpDeinit();
        return -1;
    }
    WHBLogPrint("WHBProc initialized successfully.");

    // Initialize OSScreen
    OSScreenInit();
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    WHBLogPrintf("TV buffer size: %u, DRC buffer size: %u", tvBufferSize, drcBufferSize);

    void* tvBuffer = MEM1_alloc(tvBufferSize, 0x100);
    void* drcBuffer = MEM1_alloc(drcBufferSize, 0x100);

    if (!tvBuffer || !drcBuffer) {
        WHBLogPrint("Failed to allocate screen buffers.");
        WHBProcShutdown();
        WHBLogUdpDeinit();
        return -1;
    }

    OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
    OSScreenEnableEx(SCREEN_TV, true);
    OSScreenEnableEx(SCREEN_DRC, true);
    OSScreenClearBufferEx(SCREEN_TV, 0x000000FF);
    OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF);
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
    WHBLogPrint("OSScreen buffers set successfully.");

    // Initialize SDL
    WHBLogPrint("Initializing SDL...");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        WHBLogPrintf("Failed to initialize SDL: %s", SDL_GetError());
        MEM1_free(tvBuffer);
        MEM1_free(drcBuffer);
        WHBProcShutdown();
        WHBLogUdpDeinit();
        return -1;
    }
    WHBLogPrint("SDL initialized successfully.");

    // Initialize Player
    std::string filePath = "/vol/external01/sample.mp4";
    Player player(filePath);

    if (!player.initialize()) {
        WHBLogPrint("Failed to initialize player.");
        SDL_Quit();
        MEM1_free(tvBuffer);
        MEM1_free(drcBuffer);
        WHBProcShutdown();
        WHBLogUdpDeinit();
        return -1;
    }

    WHBLogPrint("Entering main loop...");
    bool running = true;

    while (running) {
        WHBProcStatus status = WHBProcGetStatus();
        if (status == WHBProcStatus::WHBPROC_SHUTDOWN) {
            WHBLogPrint("Application shutting down...");
            running = false;
        } else if (status == WHBProcStatus::WHBPROC_RELAUNCH) {
            WHBLogPrint("Application relaunching...");
            running = false;
        } else if (status == WHBProcStatus::WHBPROC_FOCUS) {
            WHBLogPrint("Application is in foreground.");
            player.render();
        } else {
            OSScreenFlipBuffersEx(SCREEN_TV);
            OSScreenFlipBuffersEx(SCREEN_DRC);
        }
    }

    WHBLogPrint("Cleaning up resources...");
    player.stop();
    SDL_Quit();
    MEM1_free(tvBuffer);
    MEM1_free(drcBuffer);
    WHBProcShutdown();
    WHBLogUdpDeinit();

    WHBLogPrint("Returning to Wii U Menu...");
    return 0;
}
