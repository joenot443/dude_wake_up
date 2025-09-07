#ifndef PlaylistSource_hpp
#define PlaylistSource_hpp

#include "FileSource.hpp"
#include "PlaylistSettings.hpp"
#include "File.hpp"

class PlaylistSource : public FileSource {
public:
  PlaylistSource(std::string id, std::string name)
    : FileSource(id, name, "", VideoSource_playlist) {
    // Override the settings with our custom playlist settings
    settings = std::make_shared<PlaylistSettings>(id);
  }
  
  void setup() override;
  void drawSettings() override;
  
private:
  std::shared_ptr<PlaylistSettings> playlistSettings() {
    return std::static_pointer_cast<PlaylistSettings>(settings);
  }
  
  void scanFolder();
  void loadSelectedVideo();
};

#endif /* PlaylistSource_hpp */
