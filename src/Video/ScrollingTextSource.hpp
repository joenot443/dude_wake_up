//
//  ScrollingTextSource.hpp
//  dude_wake_up
//
//  Created by Claude Code
//

#ifndef ScrollingTextSource_hpp
#define ScrollingTextSource_hpp

#include "TextSource.hpp"
#include "Parameter.hpp"

class ScrollingTextSource : public TextSource {
public:
  ScrollingTextSource(std::string id, std::string name, std::shared_ptr<DisplayText> displayText)
  : TextSource(id, name, displayText),
    speed(std::make_shared<Parameter>("Speed", 100.0, 10.0, 500.0)),
    vertical(std::make_shared<Parameter>("Vertical", 0.0, ParameterType_Bool)),
    delay(std::make_shared<Parameter>("Delay", 1.0, 0.0, 5.0)),
    reverse(std::make_shared<Parameter>("Reverse", 0.0, ParameterType_Bool)),
    scrollPosition(0.0),
    isWaiting(false),
    waitStartTime(0.0),
    lastUpdateTime(0.0) {
      this->type = VideoSource_scrollingText;
    };

  void setup() override;
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;
  void drawSettings() override;

private:
  std::shared_ptr<Parameter> speed;    // Pixels per second
  std::shared_ptr<Parameter> vertical; // Scroll vertically instead of horizontally
  std::shared_ptr<Parameter> delay;    // Pause duration when off-screen
  std::shared_ptr<Parameter> reverse;  // Reverse direction (right-to-left or down-to-up)

  float scrollPosition;     // Current scroll position
  bool isWaiting;          // Whether we're in the delay period
  double waitStartTime;    // When the wait started
  double lastUpdateTime;   // Last time position was updated
};

#endif /* ScrollingTextSource_hpp */
