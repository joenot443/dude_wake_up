// AsioTest.cpp v2 - Attempt to trigger latent error by explicit use

// Include the main Asio header directly

// Explicitly include headers for types involved in the original errors


#include <iostream> // For output
#include <exception> // For exception catching
#include <ableton/Link.hpp>
#include <asio.hpp> 
#include <ableton/asio/execution/any_executor.hpp>
#include <ableton/asio/execution/prefer_only.hpp>
#include <ableton/asio/execution/outstanding_work.hpp>
#include <ableton/asio/execution/relationship.hpp>
// We might also need the type that originally failed:
#include <ableton/asio/any_completion_executor.hpp>
#include "ofMain.h"
#include "MainApp.h"
#include "VideoSource.hpp"


int main() {
    std::cout << "Starting AsioTest v2..." << std::endl;

    // 1. Instantiate Link (as before)
     ableton::Link link(120.0);
     link.captureAppSessionState();
    std::cout << "Link object created." << std::endl;

    // 2. Attempt to explicitly declare the problematic types
    //    This should force the compiler to fully resolve them.
    try {
        // Use the namespaces Asio defines
        // Note: Asio headers might be inside link_asio_1_28_0 namespace
        //       but asio.hpp usually provides aliases. Let's try asio:: first.
        using namespace link_asio_1_28_0;
        using namespace execution;

        // Declare an any_executor with the exact properties used internally
        // by any_completion_executor, which triggered errors before.
        // This forces resolution of any_executor, prefer_only, outstanding_work_t etc.
        std::cout << "Attempting to declare complex any_executor..." << std::endl;
        any_executor<
            prefer_only<outstanding_work_t::tracked_t>,
            prefer_only<outstanding_work_t::untracked_t>,
            prefer_only<relationship_t::fork_t>,
            prefer_only<relationship_t::continuation_t>
        > my_poly_executor;
        std::cout << "Declared complex any_executor." << std::endl;

        // Optionally, also try declaring the any_completion_executor type itself
        // std::cout << "Attempting to declare any_completion_executor..." << std::endl;
        // asio::any_completion_executor my_completion_executor; // Might need construction args
        // std::cout << "Declared any_completion_executor." << std::endl;


        // Simple check to potentially use the declared variable
        if (!my_poly_executor) {
             std::cout << "Default any_executor is null." << std::endl;
        }

        std::cout << "Successfully declared problematic Asio types." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Caught standard exception: " << e.what() << std::endl;
        // We don't expect exceptions here, only compile errors if hypothesis is right
    } catch (...) {
        std::cerr << "Caught unknown exception." << std::endl;
    }

    std::cout << "Finished AsioTest v2." << std::endl;
    return 0;
}