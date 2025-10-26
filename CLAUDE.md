# Nottawa - Audio-Reactive Visual Experience Platform

## Project Overview

**Nottawa** (formerly `dude_wake_up`) is a macOS application for creating real-time audio-reactive visualizations for concerts and live events. Built with OpenFrameworks and OpenGL, it provides a node-based visual programming interface where users can connect sources and effects to create dynamic visual experiences.

- **Language**: C++
- **Graphics**: OpenGL via OpenFrameworks
- **UI**: Dear ImGui with custom node editor
- **Audio**: Multi-format audio processing with BPM detection
- **License**: GPLv3

## Architecture

### Core Concept: Connectables

The application is built around a **Connectable** system where everything is either a:
- **Source** (`VideoSource`): Generates visual content (webcam, files, shaders, text, etc.)
- **Effect** (`Shader`): Processes and transforms visual content

These are connected in a node graph where data flows from Source → Effect chains, with each frame being processed through the entire chain.

### Key Components

#### 1. Connectable System (`src/Shading/Connection.hpp`)
- **Connectable**: Base class for all nodes in the graph
- **Connection**: Represents links between nodes with input/output slots
- **ConnectionType**: Shader-to-Shader or Source-to-Shader connections
- **Input/Output Slots**: Support for multiple inputs/outputs per node (slots 0-9 + auxiliary)

#### 2. Video Sources (`src/Video/`)
Available source types:
- **WebcamSource**: Live camera input
- **FileSource**: Video file playback
- **ImageSource**: Static images
- **ShaderSource**: Procedural shader-generated content
- **TextSource**: Text rendering
- **LibrarySource**: Content from shared library
- **MultiSource**: Composite multiple sources
- **PlaylistSource**: Sequential media playback

#### 3. Shader System (`src/Shading/`)
- **Shader**: Base effect class that processes frames
- **ShaderSettings**: Parameter configuration for each shader
- **ShaderType**: Classification of different effect types
- **Optional Shaders**: Additional effects that can be applied to any shader

#### 4. Audio Processing (`src/Audio/`)
- **AudioSource**: Base class for audio input (microphone, file, system audio)
- **BTrackDetector**: Real-time BPM detection using BTrack library
- **Gist**: Audio feature extraction library integration
- **AudioAnalysis**: Processed audio features for visual reactivity

#### 5. Node Graph Interface (`src/NodeLayout/`)
- **NodeLayoutView**: Main visual node editor interface
- **Node**: Visual representation of Connectables
- **ImGui Node Editor**: Custom node graph implementation
- **Real-time connection management and visual feedback

## Project Structure

```
src/
├── Apps/              # Main application entry point
├── Audio/             # Audio processing and sources
│   ├── BTrackDetector.* # BPM detection
│   ├── AudioSource.*   # Base audio input class
│   └── *AudioSource.*  # Specific audio sources
├── Models/            # Data models and structures
├── NodeLayout/        # Node graph visual interface  
├── Oscillation/       # Parameter modulation system
├── Services/          # Core application services
├── Shading/           # Shader effects system
├── Util/              # Utility classes and helpers
├── Video/             # Video sources and processing
└── Views/             # UI components and windows
```

## Key Services

All services follow a singleton pattern and are initialized in `main.cpp`:

- **AudioSourceService**: Manages audio inputs and processing
- **VideoSourceService**: Handles video sources and rendering
- **ShaderChainerService**: Manages shader effect chains
- **ModulationService**: Parameter automation and audio reactivity
- **TextureService**: OpenGL texture management
- **NodeLayoutView**: Node graph interface management
- **ConfigService**: Application configuration and settings
- **TimeService**: Timing and synchronization

## Building and Running

### Requirements
- **macOS**: Primary target platform
- **Xcode**: Used for building and running the project
- **OpenFrameworks**: Graphics and multimedia framework
- **Dependencies**: Listed in `addons.make`

### OpenFrameworks Addons
```
ofxImGui          # Dear ImGui integration
ofxMidi           # MIDI input support  
ofxSyphon         # Video sharing (macOS)
ofxFastFboReader  # Efficient framebuffer reading
ofxFFmpegRecorder # Video recording
```

### Build Configuration
- **Xcode Project**: `dude_wake_up.xcodeproj`
- **Compiler Flags**: Defined in `compile_flags.txt` for clang
- **OpenGL Version**: 3.2 Core Profile
- **C++ Standard**: C++11/17 compatible

## Audio System

### BPM Detection
The application uses the BTrack library for real-time beat detection:

```cpp
class BTrackDetector {
    SimpleBeat processFrame(Gist<float>* gist);
    SimpleBeat processAudioFrame(const std::vector<float>& frame);
    float getCurrentBpm() const;
    bool isBeatDetected() const;
};
```

### Audio Sources
- **MicrophoneAudioSource**: Live microphone input
- **FileAudioSource**: Audio file playback
- **SystemAudioSource**: System audio capture (macOS)

## Rendering Pipeline

1. **Audio Processing**: Extract features from audio input
2. **Source Generation**: Each VideoSource renders to its ofFbo
3. **Shader Processing**: Effects process frames through the connection chain
4. **Final Composite**: Render final output to main display
5. **Frame Management**: Double-buffered rendering with ofFbo objects

## Node Graph System

The visual interface allows users to:
- **Add Nodes**: Drag sources and effects from browsers
- **Create Connections**: Link outputs to inputs with visual cables
- **Real-time Preview**: See effects applied in real-time
- **Parameter Control**: Adjust settings with immediate visual feedback
- **Save/Load**: Persist node graphs as "Strands"

## Development Workflow

### Creating New Shaders
To create a new shader effect, use the hygen generator:
```bash
hygen shader new ShaderName
```
This will automatically create the necessary files and update the project configuration.

### Adding Parameters to Existing Shaders
This is a very common workflow for making shader properties controllable and audio-reactive. Follow this pattern:

**1. Update the ShaderSettings struct in the `.hpp` file:**
```cpp
struct MyShaderSettings: public ShaderSettings {
  // Add Parameter and Oscillator pairs
  std::shared_ptr<Parameter> myParam;
  std::shared_ptr<WaveformOscillator> myParamOscillator;

  MyShaderSettings(std::string shaderId, json j) :
  myParam(std::make_shared<Parameter>("My Param", defaultValue, minValue, maxValue)),
  myParamOscillator(std::make_shared<WaveformOscillator>(myParam)),
  ShaderSettings(shaderId, j, "MyShader") {
    // Register in arrays
    parameters = { myParam };
    oscillators = { myParamOscillator };
    load(j);
    registerParameters();
  };
};
```

**Important Naming Convention:**
- C++ variable names: `camelCase` (e.g., `myParam`, `redIntensity`)
- Parameter display names: `Capital Case` (e.g., `"My Param"`, `"Red Intensity"`)
- The display name is what users see in the UI, so it should be human-readable

**2. Pass the uniform to the shader in the `shade()` method:**
```cpp
void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
  canvas->begin();
  shader.begin();
  // Add uniform
  shader.setUniform1f("myParam", settings->myParam->value);
  // ... other uniforms ...
  shader.end();
  canvas->end();
}
```

**3. Add the uniform declaration in the fragment shader (`.frag` file):**
```glsl
uniform float myParam;
```
Replace any `#define` constants with the new uniform variable throughout the shader code.

**4. Add UI controls in the `drawSettings()` method:**
```cpp
void drawSettings() override {
  CommonViews::H3Title("MyShader");
  CommonViews::ShaderParameter(settings->myParam, settings->myParamOscillator);
}
```

**Parameter/Oscillator/Uniform Triple Pattern:**
- **Parameter**: Stores the value with min/max bounds and name
- **WaveformOscillator**: Enables audio-reactive modulation of the parameter
- **Uniform**: Passes the value to the GPU shader each frame

This pattern allows parameters to be:
- Manually adjusted via UI sliders
- Modulated by audio features (amplitude, frequency, etc.)
- Saved/loaded with shader presets
- Animated over time

## Development Notes

### Code Style
- Header guards: `#ifndef ClassName_hpp`
- Smart pointers: Extensive use of `std::shared_ptr`
- OpenFrameworks naming: `of` prefixed classes
- Service pattern: Singleton services for core functionality

### Key Files for Development
- `src/main.cpp`: Application entry point and service initialization
- `src/Apps/MainApp.*`: Main application loop and setup
- `src/Shading/Connection.hpp`: Core connectivity system
- `src/NodeLayout/NodeLayoutView.*`: Visual node editor
- `src/Video/VideoSource.hpp`: Base source class
- `src/Shading/Shader.hpp`: Base effect class

### Testing
- Test runner: `test.sh`
- Unit tests: `dude_wake_up_tests/` directory
- Build verification available

## Libraries and Dependencies

### Core Libraries
- **OpenFrameworks**: Primary framework
- **Dear ImGui**: UI framework with custom node editor
- **BTrack**: Beat tracking and tempo estimation  
- **Gist**: Audio feature extraction
- **nlohmann/json**: JSON serialization
- **FFTW/Kiss FFT**: Fast Fourier transforms

### Platform Integration
- **macOS AVFoundation**: System audio capture
- **Syphon**: Video sharing between applications
- **Core Audio**: Low-latency audio processing

This architecture provides a flexible, real-time visual programming environment optimized for live performance and audio-reactive content creation.