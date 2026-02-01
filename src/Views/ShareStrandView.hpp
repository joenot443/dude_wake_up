#ifndef ShareStrandView_hpp
#define ShareStrandView_hpp

//  ShareStrandView.hpp
//  dude_wake_up
//
//  Created by Claude on behalf of Joe Crozier.
//
//  Polished ImGui view for sharing strands publicly with live preview,
//  video recording with progress bar, and upload progress indication.
//  Landscape layout: preview on left, form on right.

#include "ofMain.h"
#include "json.hpp"
#include "ConfigService.hpp"
#include "ShaderChainerService.hpp"
#include "Models/Strand.hpp"
#include <fstream>
#include "imgui.h"
#include <cstring>
#include "LibraryService.hpp"
#include <functional>
#include <future>
#include <chrono>
#include "CommonViews.hpp"
#include "AVFRecorderWrapper.hpp"
#include "ofxFastFboReader.h"
#include "AudioSourceService.hpp"

enum class ShareState {
  Capturing,    // Initial state - user can capture screenshot or record video
  Recording,    // Video recording in progress
  ReadyToShare, // Content captured, ready to fill in details and share
  Uploading,    // Upload in progress
  Success       // Successfully shared
};

struct ShareStrandView {
private:
  char titleBuf[128] = {0};
  char descriptionBuf[2048] = {0};
  char authorBuf[128] = {0};

  std::string title() const { return std::string(titleBuf); }
  std::string description() const { return std::string(descriptionBuf); }
  std::string author() const { return std::string(authorBuf); }

  ShareState state = ShareState::Capturing;
  std::string error;
  std::string resultUrl;

  // The strand to share
  Strand currentStrand;
  bool hasStrand = false;

  // Capture data
  bool hasVideo = false;
  ofTexture capturedTexture;
  std::string screenshotData;
  std::string videoData;
  std::string videoFilePath;
  float videoSizeMB = 0.0f;

  // Recording
  static constexpr float MAX_VIDEO_DURATION = 15.0f;
  static constexpr float TARGET_FPS = 30.0f;
  AVFRecorderWrapper videoRecorder;
  ofPixels recordPixels;
  std::chrono::steady_clock::time_point lastFrameCaptureTime; // For frame rate limiting
  std::chrono::steady_clock::time_point recordingStartTime; // Track actual recording duration

  // Audio recording
  bool includeAudio = false;
  std::shared_ptr<AudioSource> recordingAudioSource = nullptr;
  int recordingAudioChannels = 1; // Detected from audio source

  // Upload progress animation
  float uploadAnimTime = 0.0f;

  // Link copied feedback
  float linkCopiedTime = 0.0f;

  // Layout constants
  static constexpr float PREVIEW_WIDTH = 400.0f;
  static constexpr float FORM_WIDTH = 320.0f;
  static constexpr float COLUMN_GAP = 24.0f;

  // Style constants
  const ImVec4 accentColor = ImVec4(0.30f, 0.69f, 0.71f, 1.0f);
  const ImVec4 accentHover = ImVec4(0.35f, 0.74f, 0.76f, 1.0f);
  const ImVec4 successColor = ImVec4(0.30f, 0.78f, 0.47f, 1.0f);
  const ImVec4 recordColor = ImVec4(0.90f, 0.30f, 0.30f, 1.0f);
  const ImVec4 recordColorDim = ImVec4(0.70f, 0.25f, 0.25f, 1.0f);
  const ImVec4 buttonBg = ImVec4(0.22f, 0.22f, 0.24f, 1.0f);
  const ImVec4 buttonHover = ImVec4(0.28f, 0.28f, 0.30f, 1.0f);
  const ImVec4 inputBg = ImVec4(0.14f, 0.14f, 0.16f, 1.0f);

  std::shared_ptr<Connectable> getTerminalConnectable() {
    if (currentStrand.connectables.empty()) return nullptr;
    for (auto& connectable : currentStrand.connectables) {
      auto term = connectable->terminalDescendent();
      if (term) return term;
    }
    return nullptr;
  }

  std::shared_ptr<ofFbo> getLiveFbo() {
    auto terminal = getTerminalConnectable();
    if (!terminal) return nullptr;
    return terminal->frame();
  }

  void captureScreenshot() {
    auto fbo = getLiveFbo();
    if (!fbo) {
      error = "No output to capture";
      return;
    }

    ofPixels pixels;
    fbo->getTexture().readToPixels(pixels);
    capturedTexture.loadData(pixels);

    ofBuffer buffer;
    if (ofSaveImage(pixels, buffer, OF_IMAGE_FORMAT_PNG)) {
      screenshotData = std::string(buffer.getData(), buffer.size());
      hasVideo = false;
      state = ShareState::ReadyToShare;
      error.clear();
    } else {
      error = "Failed to encode screenshot";
    }
  }

  void startVideoRecording() {
    auto fbo = getLiveFbo();
    if (!fbo) {
      error = "No output to record";
      return;
    }

    std::string timestamp = ofGetTimestampString("%Y%m%d_%H%M%S");
    videoFilePath = ConfigService::getService()->tempFilePath() + "/preview_" + timestamp + ".mp4";

    int width = fbo->getWidth();
    int height = fbo->getHeight();
    if (!videoRecorder.setup(videoFilePath, width, height, 30.0f, "h264")) {
      error = "Failed to setup video recorder";
      return;
    }

    videoRecorder.setMaxDuration(MAX_VIDEO_DURATION);

    // Initialize timing for frame rate limiting
    auto now = std::chrono::steady_clock::now();
    lastFrameCaptureTime = now - std::chrono::milliseconds(100); // Allow immediate first frame
    recordingStartTime = now;

    videoRecorder.setOnRecordingComplete([this](bool success, const std::string& path) {
      // Clean up audio recording callback
      if (recordingAudioSource) {
        recordingAudioSource->clearRecordingCallback();
        recordingAudioSource = nullptr;
      }
      if (success) {
        loadVideoData();
        hasVideo = true;
        state = ShareState::ReadyToShare;
      } else {
        error = "Video recording failed";
        state = ShareState::Capturing;
      }
    });

    // Setup audio if enabled
    if (includeAudio) {
      auto audioSource = AudioSourceService::getService()->selectedAudioSource;
      if (audioSource && audioSource->active) {
        // Get sample rate from audio source (uses actual device rate)
        float sampleRate = audioSource->getSampleRate();

        // Determine channel count based on audio source type
        int channels = 1; // Default to mono for microphone
        if (audioSource->type() == AudioSourceType_System) {
          channels = 2; // Stereo
        } else if (audioSource->type() == AudioSourceType_File) {
          channels = 2; // Stereo
        }
        // Microphone stays mono

        recordingAudioChannels = channels;

        if (videoRecorder.setupAudio(sampleRate, channels)) {
          recordingAudioSource = audioSource;

          // Set up the recording callback
          audioSource->setRecordingCallback([this, sampleRate](const float* samples, int numFrames, int numChannels, float callbackSampleRate) {
            if (videoRecorder.isRecording() && videoRecorder.hasAudio()) {
              // Log first callback to verify parameters
              static bool firstCallback = true;
              if (firstCallback) {
                ofLogNotice("ShareStrandView") << "Audio callback: numFrames=" << numFrames
                                               << ", numChannels=" << numChannels
                                               << ", callbackSampleRate=" << callbackSampleRate
                                               << ", configuredSampleRate=" << sampleRate
                                               << ", recordingAudioChannels=" << recordingAudioChannels;
                firstCallback = false;
              }

              // Handle channel count mismatch (e.g., if source changes)
              if (numChannels == recordingAudioChannels) {
                videoRecorder.addAudioSamples(samples, numFrames);
              } else if (numChannels == 1 && recordingAudioChannels == 2) {
                // Upmix mono to stereo
                std::vector<float> stereoSamples(numFrames * 2);
                for (int i = 0; i < numFrames; i++) {
                  stereoSamples[i * 2] = samples[i];
                  stereoSamples[i * 2 + 1] = samples[i];
                }
                videoRecorder.addAudioSamples(stereoSamples);
              } else if (numChannels == 2 && recordingAudioChannels == 1) {
                // Downmix stereo to mono
                std::vector<float> monoSamples(numFrames);
                for (int i = 0; i < numFrames; i++) {
                  monoSamples[i] = (samples[i * 2] + samples[i * 2 + 1]) * 0.5f;
                }
                videoRecorder.addAudioSamples(monoSamples);
              }
            }
          });
          ofLogNotice("ShareStrandView") << "Audio recording enabled: " << sampleRate << " Hz, " << channels << " channel(s)";
        } else {
          ofLogWarning("ShareStrandView") << "Failed to setup audio, recording without audio";
        }
      }
    }

    if (!videoRecorder.start()) {
      error = "Failed to start recording";
      if (recordingAudioSource) {
        recordingAudioSource->clearRecordingCallback();
        recordingAudioSource = nullptr;
      }
      return;
    }

    // Capture initial frame for thumbnail
    ofPixels pixels;
    fbo->getTexture().readToPixels(pixels);
    capturedTexture.loadData(pixels);
    ofBuffer buffer;
    if (ofSaveImage(pixels, buffer, OF_IMAGE_FORMAT_PNG)) {
      screenshotData = std::string(buffer.getData(), buffer.size());
    }

    state = ShareState::Recording;
    error.clear();
  }

  void stopVideoRecording() {
    // Clean up audio recording callback
    if (recordingAudioSource) {
      recordingAudioSource->clearRecordingCallback();
      recordingAudioSource = nullptr;
    }

    videoRecorder.stop([this](bool success) {
      if (success) {
        loadVideoData();
        hasVideo = true;
        state = ShareState::ReadyToShare;
      } else {
        error = "Video recording failed";
        state = ShareState::Capturing;
      }
    });
  }

  void loadVideoData() {
    if (videoFilePath.empty()) return;

    std::ifstream file(videoFilePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      error = "Failed to load video file";
      return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    videoData.resize(size);
    if (file.read(videoData.data(), size)) {
      videoSizeMB = size / (1024.0f * 1024.0f);
    } else {
      error = "Failed to read video file";
      videoData.clear();
    }
  }

  void cleanupVideoFile() {
    if (!videoFilePath.empty()) {
      std::remove(videoFilePath.c_str());
      videoFilePath.clear();
    }
    videoData.clear();
    videoSizeMB = 0.0f;
  }

  void resetToCapture() {
    if (state == ShareState::Recording) {
      stopVideoRecording();
    }
    cleanupVideoFile();
    screenshotData.clear();
    hasVideo = false;
    state = ShareState::Capturing;
    error.clear();
  }

  void copyToClipboard(const std::string& text) {
#ifdef TARGET_OSX
    std::string cmd = "echo '" + text + "' | pbcopy";
    system(cmd.c_str());
#endif
  }

  bool StyledButton(const char* label, ImVec2 size, ImVec4 bgColor, ImVec4 hoverColor, bool enabled = true) {
    if (!enabled) {
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }
    ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, enabled ? hoverColor : bgColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, enabled ? hoverColor : bgColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    bool pressed = ImGui::Button(label, size) && enabled;

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    if (!enabled) {
      ImGui::PopStyleVar();
    }
    return pressed;
  }

  void DrawRecordingProgress(float width) {
    float elapsed = videoRecorder.getElapsedTime();
    float progress = elapsed / MAX_VIDEO_DURATION;
    float remaining = MAX_VIDEO_DURATION - elapsed;

    float pulse = (sin(ofGetElapsedTimef() * 6.0f) + 1.0f) * 0.5f;
    ImVec4 barColor = ImVec4(
      recordColorDim.x + (recordColor.x - recordColorDim.x) * pulse,
      recordColorDim.y + (recordColor.y - recordColorDim.y) * pulse,
      recordColorDim.z + (recordColor.z - recordColorDim.z) * pulse,
      1.0f
    );

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    char overlay[32];
    snprintf(overlay, sizeof(overlay), "Recording... %.1fs", remaining > 0 ? remaining : 0);
    ImGui::ProgressBar(progress, ImVec2(width, 20), overlay);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
  }

  void DrawUploadProgress(float width) {
    uploadAnimTime += ImGui::GetIO().DeltaTime;

    float t = fmod(uploadAnimTime * 0.8f, 1.0f);
    float barStart = t - 0.3f;
    float barEnd = t + 0.1f;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float height = 20.0f;

    drawList->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height),
                            IM_COL32(25, 25, 28, 255), 4.0f);

    float startX = pos.x + width * std::max(0.0f, barStart);
    float endX = pos.x + width * std::min(1.0f, barEnd);
    if (endX > startX) {
      drawList->AddRectFilled(ImVec2(startX, pos.y + 2), ImVec2(endX, pos.y + height - 2),
                              IM_COL32(77, 176, 181, 255), 3.0f);
    }

    const char* text = "Uploading...";
    ImVec2 textSize = ImGui::CalcTextSize(text);
    drawList->AddText(ImVec2(pos.x + (width - textSize.x) * 0.5f, pos.y + (height - textSize.y) * 0.5f),
                      IM_COL32(255, 255, 255, 255), text);

    ImGui::Dummy(ImVec2(width, height));
  }

  // Draw the left column (preview + capture controls)
  void DrawPreviewColumn(float previewWidth, float previewHeight) {
    auto fbo = getLiveFbo();

    // Preview image
    ImVec2 previewPos = ImGui::GetCursorScreenPos();

    if (state == ShareState::Capturing || state == ShareState::Recording) {
      if (fbo && fbo->isAllocated()) {
        ImGui::Image((ImTextureID)(uintptr_t)fbo->getTexture().getTextureData().textureID,
                     ImVec2(previewWidth, previewHeight));
      } else {
        ImGui::Dummy(ImVec2(previewWidth, previewHeight));
      }
    } else {
      if (capturedTexture.isAllocated()) {
        ImGui::Image((ImTextureID)(uintptr_t)capturedTexture.getTextureData().textureID,
                     ImVec2(previewWidth, previewHeight));
      } else {
        ImGui::Dummy(ImVec2(previewWidth, previewHeight));
      }
    }

    // Border
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRect(previewPos, ImVec2(previewPos.x + previewWidth, previewPos.y + previewHeight),
                      IM_COL32(60, 60, 65, 255), 4.0f, 0, 2.0f);

    ImGui::Spacing();

    // Capture controls based on state
    if (state == ShareState::Capturing) {
      float buttonWidth = (previewWidth - 12) / 2;
      if (StyledButton("Screenshot", ImVec2(buttonWidth, 32), accentColor, accentHover)) {
        captureScreenshot();
      }
      ImGui::SameLine(0, 12);
      if (StyledButton("Record Video", ImVec2(buttonWidth, 32), accentColor, accentHover)) {
        startVideoRecording();
      }

      ImGui::Spacing();

      // Audio recording option
      auto audioSource = AudioSourceService::getService()->selectedAudioSource;
      bool hasActiveAudio = audioSource && audioSource->active;

      if (hasActiveAudio) {
        ImGui::Checkbox("Include Audio", &includeAudio);
        if (includeAudio) {
          ImGui::SameLine();
          ImGui::TextDisabled("(%s)", audioSource->name.c_str());
        }
      } else {
        ImGui::TextDisabled("No audio source active");
        includeAudio = false;
      }

      ImGui::Spacing();
      ImGui::TextDisabled("Video records up to 15 seconds");
    }
    else if (state == ShareState::Recording) {
      DrawRecordingProgress(previewWidth);
      ImGui::Spacing();
      float stopWidth = 120.0f;
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (previewWidth - stopWidth) / 2);
      if (StyledButton("Stop Recording", ImVec2(stopWidth, 32), recordColor, recordColorDim)) {
        stopVideoRecording();
      }
    }
    else if (state == ShareState::ReadyToShare || state == ShareState::Uploading || state == ShareState::Success) {
      // Show capture status
      ImGui::PushStyleColor(ImGuiCol_Text, successColor);
      if (hasVideo) {
        ImGui::Text("Video captured (%.1f MB)", videoSizeMB);
      } else {
        ImGui::Text("Screenshot captured");
      }
      ImGui::PopStyleColor();

      if (state == ShareState::ReadyToShare) {
        ImGui::SameLine(previewWidth - 90);
        if (StyledButton("Re-capture", ImVec2(90, 22), buttonBg, buttonHover)) {
          resetToCapture();
        }
      }
    }
  }

  // Draw the right column (form fields + actions)
  void DrawFormColumn(float formWidth) {
    if (state == ShareState::Capturing || state == ShareState::Recording) {
      // Show placeholder when not ready
      ImGui::TextDisabled("Capture a preview first");
      ImGui::Spacing();
      ImGui::TextWrapped("Use Screenshot for a static image, or Record Video for an animated preview.");
    }
    else if (state == ShareState::ReadyToShare) {
      ImGui::PushStyleColor(ImGuiCol_FrameBg, inputBg);
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));

      ImGui::Text("Title");
      ImGui::SetNextItemWidth(formWidth);
      ImGui::InputText("##title", titleBuf, IM_ARRAYSIZE(titleBuf));

      ImGui::Spacing();
      ImGui::Text("Description (optional)");
      ImGui::SetNextItemWidth(formWidth);
      ImGui::InputTextMultiline("##description", descriptionBuf, IM_ARRAYSIZE(descriptionBuf),
                                 ImVec2(formWidth, 60));

      ImGui::Spacing();
      ImGui::Text("Author (optional)");
      ImGui::SetNextItemWidth(formWidth);
      ImGui::InputText("##author", authorBuf, IM_ARRAYSIZE(authorBuf));

      ImGui::PopStyleVar(2);
      ImGui::PopStyleColor();

      if (!error.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("%s", error.c_str());
        ImGui::PopStyleColor();
      }

      ImGui::Spacing();
      ImGui::Spacing();

      // Action buttons
      bool canShare = strlen(titleBuf) > 0;
      float buttonWidth = (formWidth - 12) / 2;

      if (StyledButton("Share", ImVec2(buttonWidth, 36), accentColor, accentHover, canShare)) {
        error.clear();
        state = ShareState::Uploading;
        uploadAnimTime = 0.0f;

        std::string strandJson = currentStrand.serialize().dump();
        std::string videoToUpload = hasVideo ? videoData : "";

        LibraryService::getService()->shareStrand(
          title(), description(), strandJson, screenshotData, videoToUpload, author(),
          [this](bool ok, const std::string &url, const std::string &err) {
            if (ok) {
              resultUrl = url;
              state = ShareState::Success;
              cleanupVideoFile();
            } else {
              error = err;
              state = ShareState::ReadyToShare;
            }
          }
        );
      }

      ImGui::SameLine(0, 12);
      if (StyledButton("Cancel", ImVec2(buttonWidth, 36), buttonBg, buttonHover)) {
        cleanupVideoFile();
        hasStrand = false;
      }

      if (!canShare) {
        ImGui::Spacing();
        ImGui::TextDisabled("Enter a title to share");
      }
    }
    else if (state == ShareState::Uploading) {
      ImGui::Spacing();
      DrawUploadProgress(formWidth);
      ImGui::Spacing();
      ImGui::TextDisabled("This may take a moment...");
    }
    else if (state == ShareState::Success) {
      ImGui::PushStyleColor(ImGuiCol_Text, successColor);
      ImGui::Text("Shared successfully!");
      ImGui::PopStyleColor();

      ImGui::Spacing();
      ImGui::Spacing();

      ImGui::Text("Share URL:");
      ImGui::PushStyleColor(ImGuiCol_FrameBg, inputBg);
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));
      ImGui::SetNextItemWidth(formWidth);
      char urlBuf[512];
      strncpy(urlBuf, resultUrl.c_str(), sizeof(urlBuf) - 1);
      ImGui::InputText("##url", urlBuf, sizeof(urlBuf), ImGuiInputTextFlags_ReadOnly);
      ImGui::PopStyleVar(2);
      ImGui::PopStyleColor();

      ImGui::Spacing();
      ImGui::Spacing();

      float buttonWidth = (formWidth - 12) / 2;
      if (StyledButton("Copy Link", ImVec2(buttonWidth, 36), accentColor, accentHover)) {
        copyToClipboard(resultUrl);
        linkCopiedTime = 2.0f;
      }
      ImGui::SameLine(0, 12);
      if (StyledButton("Done", ImVec2(buttonWidth, 36), buttonBg, buttonHover)) {
        hasStrand = false;
      }

      // Show "Link Copied" feedback
      if (linkCopiedTime > 0.0f) {
        linkCopiedTime -= ImGui::GetIO().DeltaTime;
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, successColor);
        ImGui::Text("Link copied!");
        ImGui::PopStyleColor();
      }
    }
  }

public:
  void show(const Strand& strand) {
    currentStrand = strand;
    hasStrand = true;

    std::string name = strand.name.empty() ? "Untitled Strand" : strand.name;
    strncpy(titleBuf, name.c_str(), sizeof(titleBuf) - 1);
    titleBuf[sizeof(titleBuf) - 1] = '\0';
    descriptionBuf[0] = '\0';
    authorBuf[0] = '\0';

    state = ShareState::Capturing;
    error.clear();
    resultUrl.clear();
    hasVideo = false;
    screenshotData.clear();
    cleanupVideoFile();
    uploadAnimTime = 0.0f;
    linkCopiedTime = 0.0f;

    // Reset audio state but preserve preference
    if (recordingAudioSource) {
      recordingAudioSource->clearRecordingCallback();
      recordingAudioSource = nullptr;
    }
  }

  void update() {
    if (state != ShareState::Recording) return;

    // Frame rate limiting using steady_clock for accurate timing
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastFrame = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameCaptureTime).count();
    const long frameIntervalMicros = static_cast<long>(1000000.0f / TARGET_FPS); // ~33333 microseconds for 30fps

    if (timeSinceLastFrame < frameIntervalMicros) {
      return; // Skip this frame - not enough time has passed
    }
    lastFrameCaptureTime = now;

    // Log frame capture rate every second
    static int framesCaptured = 0;
    static auto lastLogTime = std::chrono::steady_clock::now();
    framesCaptured++;
    auto timeSinceLog = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastLogTime).count();
    if (timeSinceLog >= 1000) {
      ofLogNotice("ShareStrandView") << "Video capture rate: " << framesCaptured << " fps";
      framesCaptured = 0;
      lastLogTime = now;
    }

    auto fbo = getLiveFbo();
    if (!fbo) {
      stopVideoRecording();
      return;
    }

    fbo->getTexture().readToPixels(recordPixels);
    videoRecorder.addFrame(recordPixels);
  }

  void draw(bool *open = nullptr) {
    if (open && !*open) return;
    if (!hasStrand) return;

    // Calculate preview dimensions based on FBO aspect ratio
    float previewWidth = PREVIEW_WIDTH;
    float previewHeight = 225.0f; // Default 16:9

    auto fbo = getLiveFbo();
    if (fbo && fbo->isAllocated()) {
      float aspectRatio = (float)fbo->getWidth() / (float)fbo->getHeight();
      previewHeight = previewWidth / aspectRatio;
      if (previewHeight > 300.0f) {
        previewHeight = 300.0f;
        previewWidth = previewHeight * aspectRatio;
      }
    }

    float totalWidth = previewWidth + COLUMN_GAP + FORM_WIDTH + 40;

    ImGui::SetNextWindowSizeConstraints(ImVec2(totalWidth, 0), ImVec2(totalWidth, FLT_MAX));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 10));

    if (!ImGui::Begin("Share Strand", open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
      ImGui::PopStyleVar(2);
      ImGui::End();
      return;
    }

    // Header
    CommonViews::H2Title("Share Your Strand", false);
    ImGui::TextWrapped("Share this strand publicly. Anyone with the link can view and import it.");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Two-column layout
    ImGui::BeginGroup();
    DrawPreviewColumn(previewWidth, previewHeight);
    ImGui::EndGroup();

    ImGui::SameLine(0, COLUMN_GAP);

    ImGui::BeginGroup();
    DrawFormColumn(FORM_WIDTH);
    ImGui::EndGroup();

    ImGui::PopStyleVar(2);
    ImGui::End();

    // Handle close
    if (!hasStrand && open) {
      *open = false;
    }
  }

  bool isActive() const { return hasStrand; }

  void reset() {
    hasStrand = false;
    state = ShareState::Capturing;
    error.clear();
    resultUrl.clear();
    hasVideo = false;
    screenshotData.clear();
    cleanupVideoFile();
    linkCopiedTime = 0.0f;

    // Clean up audio recording
    if (recordingAudioSource) {
      recordingAudioSource->clearRecordingCallback();
      recordingAudioSource = nullptr;
    }
    includeAudio = false;
  }
};

#endif // ShareStrandView_hpp
