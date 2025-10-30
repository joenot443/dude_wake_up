//
//  TypewriterTextSource.hpp
//  dude_wake_up
//
//  Created by Claude Code
//

#ifndef TypewriterTextSource_hpp
#define TypewriterTextSource_hpp

#include "TextSource.hpp"
#include "Parameter.hpp"

class TypewriterTextSource : public TextSource {
public:
  TypewriterTextSource(std::string id, std::string name, std::shared_ptr<DisplayText> displayText)
  : TextSource(id, name, displayText),
    speed(std::make_shared<Parameter>("Speed", 20.0, 1.0, 100.0)),
    hold(std::make_shared<Parameter>("Hold", 1.0, 0.0, 5.0)),
    currentCharIndex(0),
    lastCharTime(0.0),
    isPaused(false),
    pauseStartTime(0.0) {
      this->type = VideoSource_typewriter;
    };

  void setup() override;
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;
  void drawSettings() override;

private:
  std::shared_ptr<Parameter> speed;       // Characters per second
  std::shared_ptr<Parameter> hold;        // Pause duration at end before repeating in seconds

  size_t currentCharIndex;     // Current character being displayed
  double lastCharTime;         // Last time a character was added
  bool isPaused;               // Whether we're paused at the end
  double pauseStartTime;       // When the pause started
  std::string displayedText;   // Text currently being displayed
};

#endif /* TypewriterTextSource_hpp */
