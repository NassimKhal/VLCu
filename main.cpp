#include "Player.hpp"
extern "C" {
    #include <whb/proc.h>
    #include <whb/log.h>
    #include <whb/log_udp.h>
    #include <coreinit/memory.h>
    #include <coreinit/memdefaultheap.h>
    #include <coreinit/time.h>
    #include <coreinit/thread.h>
    #include <coreinit/screen.h>
    #include <SDL2/SDL.h>
    #include <sysapp/launch.h>
}

int main(int argc, char** argv) {
    // Initialisation des logs via UDP
    WHBLogUdpInit();
    WHBLogPrint("Wii U Video Player Initializing...");

    // Initialisation de WHBProc
    WHBProcInit();
    WHBLogPrint("WHBProc initialized successfully.");

    // Initialisation de SDL
    WHBLogPrint("Initializing SDL...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        WHBLogPrintf("Failed to initialize SDL: %s", SDL_GetError());
        WHBProcShutdown();
        WHBLogUdpDeinit();
        return -1;
    }
    WHBLogPrint("SDL initialized successfully.");

    // Chargement de la vidéo
    std::string videoPath = "/vol/external01/sample.mp4";
    Player player(videoPath);

    if (!player.initialize()) {
        WHBLogPrint("Failed to initialize Player.");
        SDL_Quit();
        WHBProcShutdown();
        WHBLogUdpDeinit();
        return -1;
    }

    WHBLogPrint("Entering main loop...");
    bool running = true;

    while (running) {
        // Vérification de l'état du processus
        if (WHBProcIsRunning()) {
            WHBLogPrint("Application is running.");

            // Rendu de la vidéo
            player.render();
        } else {
            // L'utilisateur a choisi de quitter l'application
            WHBLogPrint("Application shutting down...");
            running = false;
        }

        // Pause pour éviter une boucle trop rapide
        OSSleepTicks(OSMillisecondsToTicks(16));
    }

    // Nettoyage avant de quitter
    WHBLogPrint("Cleaning up resources...");
    player.stop();
    SDL_Quit();

    // Retour au menu Wii U
    WHBLogPrint("Returning to Wii U Menu...");
    SYSLaunchMenu();
    WHBLogPrint("System menu launched.");

    return 0;
}
