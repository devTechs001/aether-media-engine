# Basic Player Example

#include <iostream>
#include <string>
#include "aether/aether.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <media-file>\n";
        return 1;
    }

    const std::string media_path = argv[1];

    // Initialize engine
    aether::Config config = aether::Config::Default();
    if (!aether::Initialize(config)) {
        std::cerr << "Failed to initialize AETHER engine\n";
        return 1;
    }

    // Create player
    auto& engine = aether::Engine::Instance();
    auto player_result = engine.CreatePlayer("BasicPlayer");
    
    if (!player_result) {
        std::cerr << "Failed to create player: " << player_result.error().message << "\n";
        aether::Shutdown();
        return 1;
    }

    auto player = std::move(player_result.value());

    // Set up callbacks
    player->OnStateChanged([](aether::PlaybackState state) {
        std::cout << "State changed: " << aether::ToString(state) << "\n";
    });

    player->OnPositionUpdate([](i64 position_ms) {
        std::cout << "\rPosition: " << position_ms << "ms" << std::flush;
    });

    // Open media
    auto open_result = player->Open(media_path);
    if (!open_result) {
        std::cerr << "Failed to open media: " << open_result.error().message << "\n";
        aether::Shutdown();
        return 1;
    }

    // Start playback
    std::cout << "Starting playback...\n";
    player->Play();

    // Wait for user input
    std::cout << "Press Enter to stop...\n";
    std::cin.get();

    // Cleanup
    player->Stop();
    engine.DestroyPlayer(player);
    aether::Shutdown();

    std::cout << "\nDone.\n";
    return 0;
}
