//
//  AvailableVideoSource.h
//  dude_wake_up
//
//  Created by Joe Crozier on 1/10/23.
//

#ifndef AvailableVideoSource_h
#define AvailableVideoSource_h
#include "LibraryFile.hpp"
#include "VideoSource.hpp"
#include "UUID.hpp"
#include "ShaderSource.hpp"

class AvailableVideoSource
{
public:
  AvailableVideoSource(std::string sourceName, std::string category, VideoSourceType type);

  virtual void generatePreview() = 0; // pure virtual function
  virtual VideoSourceType getType() const { return type; }

  std::string availableVideoSourceId;
  std::string sourceName;
  std::string category = "";
  VideoSourceType type;
  std::shared_ptr<ofTexture> preview;
  bool hasPreview;
};

class AvailableVideoSourceWebcam : public AvailableVideoSource
{
public:
  AvailableVideoSourceWebcam(std::string sourceName, int webcamId)
      : AvailableVideoSource(std::move(sourceName), "", VideoSource_webcam), webcamId(webcamId) {}
  int webcamId;

  void generatePreview() override
  {
    // webcam-specific implementation
  }
};

class AvailableVideoSourceFile : public AvailableVideoSource
{
public:
  AvailableVideoSourceFile(std::string sourceName, std::string path)
      : AvailableVideoSource(std::move(sourceName), "", VideoSource_file), path(std::move(path)) {}
  
  void generatePreview() override {};

  void generatePreview(ofVideoPlayer& videoPlayer);
  
  bool hasFailedToLoad(ofVideoPlayer& videoPlayer);
  bool canGeneratePreview(ofVideoPlayer& videoPlayer);
  
  std::string path;
};

class AvailableVideoSourceImage : public AvailableVideoSource
{
public:
  AvailableVideoSourceImage(std::string sourceName, std::string path)
      : AvailableVideoSource(std::move(sourceName), "", VideoSource_image), path(std::move(path)) {}

  void generatePreview();
  
  std::string path;
};

class AvailableVideoSourceShader : public AvailableVideoSource
{
public:
  AvailableVideoSourceShader(std::string sourceName, std::string category, ShaderSourceType shaderType)
      : AvailableVideoSource(std::move(sourceName), std::move(category), VideoSource_shader), shaderType(shaderType) {}

  void generatePreview() override;
  ShaderSourceType shaderType;

private:
};

class AvailableVideoSourceLibrary : public AvailableVideoSource
{ 
public:
  AvailableVideoSourceLibrary(std::string sourceName, std::shared_ptr<LibraryFile> libraryFile)
      : AvailableVideoSource(std::move(sourceName), libraryFile->category, VideoSource_library), libraryFile(libraryFile) {}

  std::shared_ptr<LibraryFile> libraryFile;
  void generatePreview() override
  {
    // Check if a file exists at libraryFile.thumbnailFilename
    // If it does, populate the .preview texture with the contents of that file

    if (!ofFile::doesFileExist(libraryFile->thumbnailPath()))
    {
      return;
    }

    // Only generate the preview if we haven't already
    if (!hasPreview || !preview || !preview->isAllocated()) {
      auto fbo = ofFbo();
      fbo.allocate(426, 240);
      // Read the file from libraryFile->thumbnailFilename and draw it to the fbo
      auto img = ofImage();
      img.load(libraryFile->thumbnailPath());
      if (!img.isAllocated()) {
          ofLogError("Main") << "Image not correctly loaded!";
          return;
      }
      
      fbo.begin();
      ofClear(0, 0, 0, 0);  // Clear the FBO first
      img.draw(0, 0, 426, 240);
      ofSetColor(0, 0, 0, 128);
      ofDrawRectangle(0, 0, 426, 240);
      ofSetColor(255);
      fbo.end();
      preview = std::make_shared<ofTexture>(fbo.getTexture());
      hasPreview = true;
    }
  }

private:
  std::string filename;
};

class AvailableVideoSourceText : public AvailableVideoSource
{
public:
  AvailableVideoSourceText(std::string sourceName)
      : AvailableVideoSource(std::move(sourceName), "Text", VideoSource_text) {}

  void generatePreview() override
  {
  }

private:
};

class AvailableVideoSourceIcon : public AvailableVideoSource
{
public:
  AvailableVideoSourceIcon(std::string sourceName)
      : AvailableVideoSource(std::move(sourceName), "Icon", VideoSource_icon) {}

  void generatePreview() override
  {
  }

private:
};

#endif /* AvailableVideoSource_h */
