#include "Trackers.hpp"

#include <stdexcept>

std::unique_ptr<Tracker> makeLegacyTracker();
std::unique_ptr<Tracker> makeNewTracker();

std::unique_ptr<Tracker> makeTracker(const std::string& name) {
    if (name == "legacy") return makeLegacyTracker();
    if (name == "new") return makeNewTracker();
    throw std::runtime_error("unknown tracker: " + name);
}
