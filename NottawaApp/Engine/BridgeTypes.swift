//
//  BridgeTypes.swift
//  NottawaApp
//
//  Swift equivalents of the C bridge structs with automatic memory management.
//

import Foundation

// MARK: - Parameter

// MARK: - WaveShape

enum WaveShape: Int, CaseIterable, Identifiable {
    case sine = 0
    case square = 1
    case sawtooth = 2
    case triangle = 3
    case pulseTrain = 4
    case exponentialSine = 5
    case harmonicStack = 6
    case rectifiedSine = 7
    case noiseModulatedSine = 8
    case bitcrushedSine = 9
    case moireOscillation = 10

    var id: Int { rawValue }

    var displayName: String {
        switch self {
        case .sine: return "Sine"
        case .square: return "Square"
        case .sawtooth: return "Sawtooth"
        case .triangle: return "Triangle"
        case .pulseTrain: return "Pulse Train"
        case .exponentialSine: return "Exp Sine"
        case .harmonicStack: return "Harmonic"
        case .rectifiedSine: return "Rectified"
        case .noiseModulatedSine: return "Noise Sine"
        case .bitcrushedSine: return "Bitcrushed"
        case .moireOscillation: return "Moire"
        }
    }
}

// MARK: - Audio Driver Option

struct AudioDriverOption: Identifiable, Hashable {
    let name: String
    let index: Int
    var id: Int { index }
}

struct ParameterInfo: Identifiable, Hashable {
    let id: String
    let name: String
    var value: Float
    var minValue: Float
    var maxValue: Float
    var parameterType: ParameterType
    var intValue: Int
    var boolValue: Bool
    var isFavorited: Bool
    var color: (r: Float, g: Float, b: Float, a: Float)

    // Oscillator
    var hasOscillator: Bool
    var oscillatorEnabled: Bool
    var oscillatorFrequency: Float
    var oscillatorMinOutput: Float
    var oscillatorMaxOutput: Float
    var oscillatorWaveShape: WaveShape

    // Audio driver
    var hasDriver: Bool
    var driverName: String?
    var driverShift: Float
    var driverScale: Float

    enum ParameterType: Int {
        case standard = 0
        case int_ = 1
        case bool_ = 2
        case color = 3
        case hidden = 4
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }

    static func == (lhs: ParameterInfo, rhs: ParameterInfo) -> Bool {
        lhs.id == rhs.id
    }

    init(from c: NTWParameterInfo) {
        self.id = String(cString: c.id)
        self.name = String(cString: c.name)
        self.value = c.value
        self.minValue = c.minValue
        self.maxValue = c.maxValue
        self.parameterType = ParameterType(rawValue: Int(c.parameterType)) ?? .standard
        self.intValue = Int(c.intValue)
        self.boolValue = c.boolValue != 0
        self.isFavorited = c.isFavorited != 0
        self.color = (c.colorR, c.colorG, c.colorB, c.colorA)

        self.hasOscillator = c.hasOscillator != 0
        self.oscillatorEnabled = c.oscillatorEnabled != 0
        self.oscillatorFrequency = c.oscillatorFrequency
        self.oscillatorMinOutput = c.oscillatorMinOutput
        self.oscillatorMaxOutput = c.oscillatorMaxOutput
        self.oscillatorWaveShape = WaveShape(rawValue: Int(c.oscillatorWaveShape)) ?? .sine

        self.hasDriver = c.hasDriver != 0
        self.driverName = c.driverName != nil ? String(cString: c.driverName) : nil
        self.driverShift = c.driverShift
        self.driverScale = c.driverScale
    }
}

// MARK: - Connectable (Shader or Video Source node)

struct ConnectableInfo: Identifiable, Hashable {
    let id: String
    let name: String
    let shaderTypeRaw: Int
    let connectableType: ConnectableType
    let inputCount: Int
    var posX: Float
    var posY: Float

    enum ConnectableType: Int {
        case source = 0
        case shader = 1
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }

    static func == (lhs: ConnectableInfo, rhs: ConnectableInfo) -> Bool {
        lhs.id == rhs.id
    }

    init(from c: NTWConnectableInfo) {
        self.id = String(cString: c.id)
        self.name = String(cString: c.name)
        self.shaderTypeRaw = Int(c.shaderTypeRaw)
        self.connectableType = ConnectableType(rawValue: Int(c.connectableType)) ?? .shader
        self.inputCount = Int(c.inputCount)
        self.posX = c.posX
        self.posY = c.posY
    }
}

// MARK: - Connection

struct ConnectionInfo: Identifiable, Hashable {
    let id: String
    let startId: String
    let endId: String
    let connectionType: ConnectionType
    let inputSlot: Int
    let outputSlot: Int

    enum ConnectionType: Int {
        case shader = 0
        case source = 1
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }

    static func == (lhs: ConnectionInfo, rhs: ConnectionInfo) -> Bool {
        lhs.id == rhs.id
    }

    init(from c: NTWConnectionInfo) {
        self.id = String(cString: c.id)
        self.startId = String(cString: c.startId)
        self.endId = String(cString: c.endId)
        self.connectionType = ConnectionType(rawValue: Int(c.connectionType)) ?? .shader
        self.inputSlot = Int(c.inputSlot)
        self.outputSlot = Int(c.outputSlot)
    }
}

// MARK: - Available Shader (for browser)

struct AvailableShaderInfo: Identifiable, Hashable {
    let id: String
    let name: String
    let category: String?
    let shaderTypeRaw: Int

    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }

    static func == (lhs: AvailableShaderInfo, rhs: AvailableShaderInfo) -> Bool {
        lhs.id == rhs.id
    }

    init(from c: NTWAvailableShaderInfo) {
        self.id = String(cString: c.id)
        self.name = String(cString: c.name)
        self.category = c.category != nil ? String(cString: c.category) : nil
        self.shaderTypeRaw = Int(c.shaderTypeRaw)
    }
}

// MARK: - Video Source

struct VideoSourceInfo: Identifiable, Hashable {
    let id: String
    let name: String
    let sourceType: Int
    let inputCount: Int
    var posX: Float
    var posY: Float

    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }

    static func == (lhs: VideoSourceInfo, rhs: VideoSourceInfo) -> Bool {
        lhs.id == rhs.id
    }

    init(from c: NTWVideoSourceInfo) {
        self.id = String(cString: c.id)
        self.name = String(cString: c.name)
        self.sourceType = Int(c.sourceType)
        self.inputCount = Int(c.inputCount)
        self.posX = c.posX
        self.posY = c.posY
    }
}

// MARK: - Audio Source

struct AudioSourceInfo: Identifiable, Hashable {
    let id: String
    let name: String
    let sourceType: Int

    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }

    static func == (lhs: AudioSourceInfo, rhs: AudioSourceInfo) -> Bool {
        lhs.id == rhs.id
    }

    init(from c: NTWAudioSourceInfo) {
        self.id = String(cString: c.id)
        self.name = String(cString: c.name)
        self.sourceType = Int(c.sourceType)
    }
}

// MARK: - Active Oscillator

struct ActiveOscillatorInfo: Identifiable {
    let paramId: String
    let paramName: String
    let ownerName: String
    let connectableId: String
    let enabled: Bool
    let frequency: Float
    let minOutput: Float
    let maxOutput: Float
    let waveShape: WaveShape
    let paramMin: Float
    let paramMax: Float
    let currentValue: Float

    var id: String { paramId }

    init(from c: NTWActiveOscillatorInfo) {
        self.paramId = String(cString: c.paramId)
        self.paramName = String(cString: c.paramName)
        self.ownerName = String(cString: c.ownerName)
        self.connectableId = String(cString: c.connectableId)
        self.enabled = c.oscillatorEnabled != 0
        self.frequency = c.frequency
        self.minOutput = c.minOutput
        self.maxOutput = c.maxOutput
        self.waveShape = WaveShape(rawValue: Int(c.waveShape)) ?? .sine
        self.paramMin = c.paramMin
        self.paramMax = c.paramMax
        self.currentValue = c.currentValue
    }
}

// MARK: - Library File

struct LibraryFileInfo: Identifiable, Hashable {
    let id: String
    let name: String
    let category: String
    let thumbnailPath: String?
    var isDownloaded: Bool
    var isDownloading: Bool
    var isPaused: Bool
    var downloadProgress: Float

    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }

    static func == (lhs: LibraryFileInfo, rhs: LibraryFileInfo) -> Bool {
        lhs.id == rhs.id
    }

    init(from c: NTWLibraryFileInfo) {
        self.id = String(cString: c.id)
        self.name = String(cString: c.name)
        self.category = String(cString: c.category)
        let path = String(cString: c.thumbnailPath)
        self.thumbnailPath = path.isEmpty ? nil : path
        self.isDownloaded = c.isDownloaded != 0
        self.isDownloading = c.isDownloading != 0
        self.isPaused = c.isPaused != 0
        self.downloadProgress = c.downloadProgress
    }
}

// MARK: - Library Source State (for download overlay on nodes)

struct LibrarySourceState {
    let state: Int          // 0=Waiting, 1=Downloading, 2=Completed, 3=Failed
    let progress: Float
    let isPaused: Bool
    let libraryFileId: String?

    var isDownloading: Bool { state == 0 || state == 1 }
    var isCompleted: Bool { state == 2 }
}

// MARK: - File Source State

struct FileSourceState {
    let volume: Float
    let speed: Float
    let position: Float
    let duration: Float
    let isMuted: Bool
    let isPlaying: Bool
}

// MARK: - Audio Analysis

struct AudioAnalysisSnapshot {
    let rms: Float
    let lows: Float
    let mids: Float
    let highs: Float
    let bpm: Float
    let beatPulse: Float
    let beatDetected: Bool

    init(from c: NTWAudioAnalysisInfo) {
        self.rms = c.rms
        self.lows = c.lows
        self.mids = c.mids
        self.highs = c.highs
        self.bpm = c.bpm
        self.beatPulse = c.beatPulse
        self.beatDetected = c.beatDetected
    }

    static let zero = AudioAnalysisSnapshot(
        from: NTWAudioAnalysisInfo(
            rms: 0, lows: 0, mids: 0, highs: 0,
            bpm: 120, beatPulse: 0, beatDetected: false
        )
    )
}
