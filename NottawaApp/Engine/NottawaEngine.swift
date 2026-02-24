//
//  NottawaEngine.swift
//  NottawaApp
//
//  Swift wrapper around the C bridge API. Provides a safe, idiomatic
//  Swift interface to the C++ engine. All C memory management is handled
//  here so callers never touch raw pointers.
//

import Foundation
import IOSurface

@Observable
final class NottawaEngine {
    static let shared = NottawaEngine()

    private(set) var isInitialized = false
    private(set) var isRunning = false

    private init() {}

    // MARK: - Lifecycle

    func initialize() -> Bool {
        guard !isInitialized else { return true }
        let result = ntw_engine_init()
        isInitialized = result
        return result
    }

    func start() {
        guard isInitialized, !isRunning else { return }
        ntw_engine_start()
        isRunning = true
    }

    func stop() {
        guard isRunning else { return }
        ntw_engine_stop()
        isRunning = false
    }

    func tick() {
        ntw_engine_tick()
    }

    /// Engine ticks per second (C++ render loop).
    var engineFPS: Float { ntw_engine_fps() }

    // MARK: - Shader Mutations

    @discardableResult
    func addShader(type: Int) -> String? {
        guard let cStr = ntw_add_shader(Int32(type)) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    func removeShader(id: String) {
        ntw_remove_shader(id)
    }

    // MARK: - Video Source Mutations

    @discardableResult
    func addShaderVideoSource(type: Int) -> String? {
        guard let cStr = ntw_add_shader_video_source(Int32(type)) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    @discardableResult
    func addTextVideoSource(name: String) -> String? {
        guard let cStr = ntw_add_text_video_source(name) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    @discardableResult
    func addImageVideoSource(name: String, path: String) -> String? {
        guard let cStr = ntw_add_image_video_source(name, path) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    @discardableResult
    func addFileVideoSource(name: String, path: String) -> String? {
        guard let cStr = ntw_add_file_video_source(name, path) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    @discardableResult
    func addWebcamVideoSource(name: String, deviceId: Int) -> String? {
        guard let cStr = ntw_add_webcam_video_source(name, Int32(deviceId)) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    @discardableResult
    func addIconVideoSource(name: String) -> String? {
        guard let cStr = ntw_add_icon_video_source(name) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    @discardableResult
    func addPlaylistVideoSource(name: String) -> String? {
        guard let cStr = ntw_add_playlist_video_source(name) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    func removeVideoSource(id: String) {
        ntw_remove_video_source(id)
    }

    // MARK: - Connection Mutations

    @discardableResult
    func makeConnection(
        startId: String,
        endId: String,
        connectionType: Int,
        outputSlot: Int,
        inputSlot: Int
    ) -> String? {
        guard let cStr = ntw_make_connection(
            startId, endId,
            Int32(connectionType),
            Int32(outputSlot),
            Int32(inputSlot)
        ) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    func removeConnection(id: String) {
        ntw_remove_connection(id)
    }

    // MARK: - Undo / Redo

    func undo() { ntw_undo() }
    func redo() { ntw_redo() }
    var canUndo: Bool { ntw_can_undo() }
    var canRedo: Bool { ntw_can_redo() }

    // MARK: - Queries

    func allShaders() -> [ConnectableInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_all_shaders(&count), count > 0 else { return [] }
        defer { ntw_free_connectable_info_array(ptr, count) }
        return (0..<Int(count)).map { ConnectableInfo(from: ptr[$0]) }
    }

    func shader(id: String) -> ConnectableInfo? {
        var info = ntw_get_shader(id)
        guard info.id != nil else { return nil }
        defer { ntw_free_connectable_info(&info) }
        return ConnectableInfo(from: info)
    }

    func allVideoSources() -> [VideoSourceInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_all_video_sources(&count), count > 0 else { return [] }
        defer { ntw_free_video_source_info_array(ptr, count) }
        return (0..<Int(count)).map { VideoSourceInfo(from: ptr[$0]) }
    }

    func allConnections() -> [ConnectionInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_all_connections(&count), count > 0 else { return [] }
        defer { ntw_free_connection_info_array(ptr, count) }
        return (0..<Int(count)).map { ConnectionInfo(from: ptr[$0]) }
    }

    func parameters(for connectableId: String) -> [ParameterInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_parameters(connectableId, &count), count > 0 else { return [] }
        defer { ntw_free_parameter_info_array(ptr, count) }
        return (0..<Int(count)).map { ParameterInfo(from: ptr[$0]) }
    }

    // MARK: - Parameter Control

    func setParameterValue(paramId: String, value: Float) {
        ntw_set_parameter_value(paramId, value)
    }

    func setParameterInt(paramId: String, value: Int) {
        ntw_set_parameter_int(paramId, Int32(value))
    }

    func setParameterBool(paramId: String, value: Bool) {
        ntw_set_parameter_bool(paramId, value)
    }

    func setParameterColor(paramId: String, r: Float, g: Float, b: Float, a: Float) {
        ntw_set_parameter_color(paramId, r, g, b, a)
    }

    func toggleParameterFavorite(paramId: String) {
        ntw_toggle_parameter_favorite(paramId)
    }

    // MARK: - Oscillator Control

    func setOscillatorEnabled(paramId: String, enabled: Bool) {
        ntw_set_oscillator_enabled(paramId, enabled)
    }

    func setOscillatorFrequency(paramId: String, frequency: Float) {
        ntw_set_oscillator_frequency(paramId, frequency)
    }

    func setOscillatorMinOutput(paramId: String, minOutput: Float) {
        ntw_set_oscillator_min_output(paramId, minOutput)
    }

    func setOscillatorMaxOutput(paramId: String, maxOutput: Float) {
        ntw_set_oscillator_max_output(paramId, maxOutput)
    }

    func setOscillatorWaveShape(paramId: String, waveShape: Int) {
        ntw_set_oscillator_wave_shape(paramId, Int32(waveShape))
    }

    // MARK: - Audio Driver Control

    func setParameterDriver(paramId: String, audioParamName: String) {
        ntw_set_parameter_driver(paramId, audioParamName)
    }

    func removeParameterDriver(paramId: String) {
        ntw_remove_parameter_driver(paramId)
    }

    func setParameterDriverShift(paramId: String, shift: Float) {
        ntw_set_parameter_driver_shift(paramId, shift)
    }

    func setParameterDriverScale(paramId: String, scale: Float) {
        ntw_set_parameter_driver_scale(paramId, scale)
    }

    func availableAudioDrivers() -> [AudioDriverOption] {
        var count: Int32 = 0
        guard let ptr = ntw_get_available_audio_drivers(&count), count > 0 else { return [] }
        defer { ntw_free_audio_driver_param_array(ptr, count) }
        return (0..<Int(count)).map {
            AudioDriverOption(
                name: String(cString: ptr[$0].name),
                index: Int(ptr[$0].index)
            )
        }
    }

    // MARK: - Available Shaders

    enum ShaderCategory: Int {
        case all = 0
        case basic = 1
        case mix = 2
        case transform = 3
        case filter = 4
        case mask = 5
        case glitch = 6
    }

    func availableShaders(category: ShaderCategory = .all) -> [AvailableShaderInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_available_shaders(Int32(category.rawValue), &count),
              count > 0 else { return [] }
        defer { ntw_free_available_shader_info_array(ptr, count) }
        return (0..<Int(count)).map { AvailableShaderInfo(from: ptr[$0]) }
    }

    // MARK: - Position & Active State

    func setConnectablePosition(id: String, x: Float, y: Float) {
        ntw_set_connectable_position(id, x, y)
    }

    func setConnectableActive(id: String, active: Bool) {
        ntw_set_connectable_active(id, active)
    }

    func isConnectableActive(id: String) -> Bool {
        ntw_get_connectable_active(id)
    }

    func removeConnectable(id: String) {
        ntw_remove_connectable(id)
    }

    // MARK: - Blend / Replace / Shader File

    @discardableResult
    func createBlendBetween(id1: String, id2: String) -> String? {
        guard let cStr = ntw_create_blend_between(id1, id2) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    @discardableResult
    func replaceShader(shaderId: String, newType: Int) -> String? {
        guard let cStr = ntw_replace_shader(shaderId, Int32(newType)) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    @discardableResult
    func replaceVideoSource(sourceId: String, newType: Int) -> String? {
        guard let cStr = ntw_replace_video_source(sourceId, Int32(newType)) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    func shaderFilePath(shaderId: String) -> String? {
        guard let cStr = ntw_get_shader_file_path(shaderId) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    func supportsAuxOutput(shaderId: String) -> Bool {
        ntw_supports_aux_output(shaderId)
    }

    // MARK: - Connections for Connectable

    func connections(for connectableId: String) -> [ConnectionInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_connections_for_connectable(connectableId, &count), count > 0 else { return [] }
        defer { ntw_free_connection_info_array(ptr, count) }
        return (0..<Int(count)).map { ConnectionInfo(from: ptr[$0]) }
    }

    // MARK: - Strand Operations

    @discardableResult
    func saveStrand(nodeId: String, path: String, name: String) -> Bool {
        ntw_save_strand_from_node(nodeId, path, name)
    }

    func availableStrands() -> [StrandInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_available_strands(&count), count > 0 else { return [] }
        defer { ntw_free_available_strand_info_array(ptr, count) }
        return (0..<Int(count)).map { StrandInfo(from: ptr[$0]) }
    }

    func loadStrand(id: String) -> [String] {
        var count: Int32 = 0
        guard let ptr = ntw_load_strand(id, &count), count > 0 else { return [] }
        defer { ntw_free_string_array(ptr, count) }
        return (0..<Int(count)).compactMap { idx in
            guard let cStr = ptr[idx] else { return nil }
            return String(cString: cStr)
        }
    }

    func availableTemplateStrands() -> [StrandInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_available_template_strands(&count), count > 0 else { return [] }
        defer { ntw_free_available_strand_info_array(ptr, count) }
        return (0..<Int(count)).map { StrandInfo(from: ptr[$0]) }
    }

    func loadDemoStrand() -> [String] {
        var count: Int32 = 0
        guard let ptr = ntw_load_demo_strand(&count), count > 0 else { return [] }
        defer { ntw_free_string_array(ptr, count) }
        return (0..<Int(count)).compactMap { idx in
            guard let cStr = ptr[idx] else { return nil }
            return String(cString: cStr)
        }
    }

    func deleteStrand(id: String) {
        ntw_delete_strand(id)
    }

    func renameStrand(id: String, newName: String) {
        ntw_rename_strand(id, newName)
    }

    func registerStrandsUpdatedCallback(_ callback: @escaping () -> Void) {
        strandsUpdatedHandler = callback
        ntw_register_strands_updated_callback {
            DispatchQueue.main.async {
                NottawaEngine.shared.strandsUpdatedHandler?()
            }
        }
    }

    private var strandsUpdatedHandler: (() -> Void)?

    // MARK: - Strand Sharing & Community

    // Stored closures for async C callbacks (C function pointers can't capture)
    private var pendingShareCompletion: ((Result<String, Error>) -> Void)?
    private var pendingFeedCompletion: (([SharedStrandInfo], Int) -> Void)?
    private var pendingVoteCompletion: ((Bool, Int, Int, Int, String?) -> Void)?
    private var pendingFetchCompletion: ((Result<String, Error>) -> Void)?

    func shareStrand(nodeId: String, title: String, description: String, author: String,
                     previewPng: Data?, completion: @escaping (Result<String, Error>) -> Void) {
        pendingShareCompletion = completion

        if let data = previewPng {
            data.withUnsafeBytes { rawBuf in
                ntw_share_strand(nodeId, title, description, author,
                                rawBuf.baseAddress, Int32(data.count)) { success, slugOrError in
                    DispatchQueue.main.async {
                        guard let cb = NottawaEngine.shared.pendingShareCompletion else { return }
                        NottawaEngine.shared.pendingShareCompletion = nil
                        if success, let s = slugOrError {
                            cb(.success(String(cString: s)))
                        } else {
                            let msg = slugOrError != nil ? String(cString: slugOrError!) : "Unknown error"
                            cb(.failure(NSError(domain: "NTW", code: -1,
                                               userInfo: [NSLocalizedDescriptionKey: msg])))
                        }
                    }
                }
            }
        } else {
            ntw_share_strand(nodeId, title, description, author, nil, 0) { success, slugOrError in
                DispatchQueue.main.async {
                    guard let cb = NottawaEngine.shared.pendingShareCompletion else { return }
                    NottawaEngine.shared.pendingShareCompletion = nil
                    if success, let s = slugOrError {
                        cb(.success(String(cString: s)))
                    } else {
                        let msg = slugOrError != nil ? String(cString: slugOrError!) : "Unknown error"
                        cb(.failure(NSError(domain: "NTW", code: -1,
                                           userInfo: [NSLocalizedDescriptionKey: msg])))
                    }
                }
            }
        }
    }

    func fetchCommunityFeed(page: Int, limit: Int, voterId: String,
                            completion: @escaping ([SharedStrandInfo], Int) -> Void) {
        pendingFeedCompletion = completion
        ntw_fetch_community_feed(Int32(page), Int32(limit), voterId) { success, ptr, count, totalPages in
            DispatchQueue.main.async {
                guard let cb = NottawaEngine.shared.pendingFeedCompletion else { return }
                NottawaEngine.shared.pendingFeedCompletion = nil
                guard success, let ptr = ptr, count > 0 else {
                    cb([], Int(totalPages))
                    return
                }
                let strands = (0..<Int(count)).map { SharedStrandInfo(from: ptr[$0]) }
                ntw_free_shared_strand_info_array(ptr, count)
                cb(strands, Int(totalPages))
            }
        }
    }

    func voteOnStrand(slug: String, voterId: String, vote: String,
                      completion: @escaping (Bool, Int, Int, Int, String?) -> Void) {
        pendingVoteCompletion = completion
        ntw_vote_strand(slug, voterId, vote) { success, upvotes, downvotes, score, userVote in
            DispatchQueue.main.async {
                guard let cb = NottawaEngine.shared.pendingVoteCompletion else { return }
                NottawaEngine.shared.pendingVoteCompletion = nil
                let voteStr: String? = userVote != nil ? String(cString: userVote!) : nil
                cb(success, Int(upvotes), Int(downvotes), Int(score),
                   (voteStr?.isEmpty == true) ? nil : voteStr)
            }
        }
    }

    func fetchSharedStrand(slug: String, completion: @escaping (Result<String, Error>) -> Void) {
        pendingFetchCompletion = completion
        ntw_fetch_shared_strand(slug) { success, jsonOrError in
            DispatchQueue.main.async {
                guard let cb = NottawaEngine.shared.pendingFetchCompletion else { return }
                NottawaEngine.shared.pendingFetchCompletion = nil
                if success, let s = jsonOrError {
                    cb(.success(String(cString: s)))
                } else {
                    let msg = jsonOrError != nil ? String(cString: jsonOrError!) : "Unknown error"
                    cb(.failure(NSError(domain: "NTW", code: -1,
                                       userInfo: [NSLocalizedDescriptionKey: msg])))
                }
            }
        }
    }

    func loadStrandFromJson(_ json: String) -> [String] {
        var count: Int32 = 0
        guard let ptr = ntw_load_strand_from_json(json, &count), count > 0 else { return [] }
        defer { ntw_free_string_array(ptr, count) }
        return (0..<Int(count)).compactMap { idx in
            guard let cStr = ptr[idx] else { return nil }
            return String(cString: cStr)
        }
    }

    func getStrandJson(nodeId: String) -> String? {
        guard let cStr = ntw_get_strand_json_for_node(nodeId) else { return nil }
        defer { ntw_free_string(cStr) }
        return String(cString: cStr)
    }

    // MARK: - Optional Shaders

    struct OptionalShaderInfo {
        let name: String
        let enabled: Bool
        let index: Int
    }

    func optionalShaders(for connectableId: String) -> [OptionalShaderInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_optional_shaders(connectableId, &count), count > 0 else { return [] }
        defer { ntw_free_optional_shader_info_array(ptr, count) }
        return (0..<Int(count)).map {
            OptionalShaderInfo(
                name: String(cString: ptr[$0].name),
                enabled: ptr[$0].enabled,
                index: Int(ptr[$0].index)
            )
        }
    }

    func toggleOptionalShader(connectableId: String, index: Int) {
        ntw_toggle_optional_shader(connectableId, Int32(index))
    }

    func optionalShaderParameters(connectableId: String, optionalIndex: Int) -> [ParameterInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_optional_shader_parameters(connectableId, Int32(optionalIndex), &count),
              count > 0 else { return [] }
        defer { ntw_free_parameter_info_array(ptr, count) }
        return (0..<Int(count)).map { ParameterInfo(from: ptr[$0]) }
    }

    // MARK: - Preview Management

    @discardableResult
    func createPreview(shaderTypeRaw: Int, isSource: Bool) -> String? {
        guard let cStr = ntw_create_preview(Int32(shaderTypeRaw), isSource) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    func destroyPreview(id: String) {
        ntw_destroy_preview(id)
    }

    func destroyAllPreviews() {
        ntw_destroy_all_previews()
    }

    // MARK: - Available Shader Sources

    func availableShaderSources() -> [AvailableShaderInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_available_shader_sources(&count), count > 0 else { return [] }
        defer { ntw_free_available_shader_info_array(ptr, count) }
        return (0..<Int(count)).map { AvailableShaderInfo(from: ptr[$0]) }
    }

    // MARK: - Selection

    func selectConnectable(id: String) {
        ntw_select_connectable(id)
    }

    func deselectConnectable() {
        ntw_deselect_connectable()
    }

    var selectedConnectableId: String? {
        guard let cStr = ntw_get_selected_connectable_id() else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    // MARK: - Active Oscillators

    func activeOscillators() -> [ActiveOscillatorInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_active_oscillators(&count), count > 0 else { return [] }
        defer { ntw_free_active_oscillator_array(ptr, count) }
        return (0..<Int(count)).map { ActiveOscillatorInfo(from: ptr[$0]) }
    }

    // MARK: - Audio

    func audioAnalysis() -> AudioAnalysisSnapshot {
        AudioAnalysisSnapshot(from: ntw_get_audio_analysis())
    }

    var currentBPM: Float {
        ntw_get_current_bpm()
    }

    func allAudioSources() -> [AudioSourceInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_all_audio_sources(&count), count > 0 else { return [] }
        defer { ntw_free_audio_source_info_array(ptr, count) }
        return (0..<Int(count)).map { AudioSourceInfo(from: ptr[$0]) }
    }

    func selectAudioSource(id: String) {
        ntw_select_audio_source(id)
    }

    // MARK: - Audio Panel

    func extendedAudioAnalysis() -> ExtendedAudioAnalysisSnapshot {
        ExtendedAudioAnalysisSnapshot(from: ntw_get_extended_audio_analysis())
    }

    func sampleTracks() -> [AudioTrackInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_sample_tracks(&count), count > 0 else { return [] }
        defer { ntw_free_audio_track_info_array(ptr, count) }
        return (0..<Int(count)).map {
            AudioTrackInfo(
                name: String(cString: ptr[$0].name),
                path: String(cString: ptr[$0].path),
                bpm: Int(ptr[$0].bpm),
                index: Int(ptr[$0].index)
            )
        }
    }

    func fileAudioState() -> FileAudioState {
        let s = ntw_get_file_audio_state()
        return FileAudioState(
            volume: s.volume,
            isPaused: s.isPaused,
            playbackPosition: s.playbackPosition,
            totalDuration: s.totalDuration,
            isFileSource: s.isFileSource,
            selectedTrackIndex: Int(s.selectedTrackIndex)
        )
    }

    var selectedAudioSourceId: String? {
        guard let cStr = ntw_get_selected_audio_source_id() else { return nil }
        defer { ntw_free_string(UnsafeMutablePointer(mutating: cStr)) }
        return String(cString: cStr)
    }

    func toggleAudioSource() { ntw_toggle_audio_source() }
    func setBpmMode(_ mode: Int) { ntw_set_bpm_mode(Int32(mode)) }
    func setBpmLocked(_ locked: Bool) { ntw_set_bpm_locked(locked) }
    func setBpmValue(_ bpm: Float) { ntw_set_bpm_value(bpm) }
    func nudgeBpm(_ delta: Float) { ntw_nudge_bpm(delta) }
    func setBpmNudge(_ value: Float) { ntw_set_bpm_nudge(value) }
    func tapBpm() { ntw_tap_bpm() }
    func setBpmEnabled(_ enabled: Bool) { ntw_set_bpm_enabled(enabled) }
    func setSmoothingMode(_ mode: Int) { ntw_set_smoothing_mode(Int32(mode)) }
    func setFrequencyRelease(_ value: Float) { ntw_set_frequency_release(value) }
    func setFrequencyScale(_ value: Float) { ntw_set_frequency_scale(value) }
    func setLoudnessRelease(_ value: Float) { ntw_set_loudness_release(value) }
    func selectSampleTrack(_ index: Int) { ntw_select_sample_track(Int32(index)) }
    func setFileAudioVolume(_ volume: Float) { ntw_set_file_audio_volume(volume) }
    func toggleFileAudioPause() { ntw_toggle_file_audio_pause() }
    func setFileAudioPosition(_ position: Float) { ntw_set_file_audio_position(position) }

    // MARK: - Screenshot

    func captureScreenshot() { ntw_capture_screenshot() }

    // MARK: - Config

    func saveDefaultConfig() { ntw_save_default_config() }
    func loadDefaultConfig() { ntw_load_default_config() }
    func saveWorkspace() { ntw_save_workspace() }

    @discardableResult
    func loadWorkspace(path: String) -> Bool {
        ntw_load_workspace(path)
    }

    func clearGraph() {
        ntw_clear_graph()
    }

    func saveWorkspaceToPath(path: String, name: String) {
        ntw_save_workspace_to_path(path, name)
    }

    var currentWorkspacePath: String? {
        guard let cStr = ntw_get_current_workspace_path() else { return nil }
        defer { ntw_free_string(cStr) }
        return String(cString: cStr)
    }

    var currentWorkspaceName: String? {
        guard let cStr = ntw_get_current_workspace_name() else { return nil }
        defer { ntw_free_string(cStr) }
        return String(cString: cStr)
    }

    // MARK: - Resolution

    func setResolution(settingIndex: Int) {
        ntw_set_resolution(Int32(settingIndex))
    }

    var resolutionSettingIndex: Int {
        Int(ntw_get_resolution_setting_index())
    }

    func setCustomResolution(width: Float, height: Float) {
        ntw_set_custom_resolution(width, height)
    }

    var resolutionWidth: Float { ntw_get_resolution_width() }
    var resolutionHeight: Float { ntw_get_resolution_height() }

    // MARK: - Callbacks

    func registerGraphChangedCallback(_ callback: @escaping () -> Void) {
        // Store the closure so it stays alive.
        graphChangedHandler = callback
        ntw_register_graph_changed_callback {
            // Dispatch to main thread for SwiftUI.
            DispatchQueue.main.async {
                NottawaEngine.shared.graphChangedHandler?()
            }
        }
    }

    func registerResolutionChangedCallback(_ callback: @escaping () -> Void) {
        resolutionChangedHandler = callback
        ntw_register_resolution_changed_callback {
            DispatchQueue.main.async {
                NottawaEngine.shared.resolutionChangedHandler?()
            }
        }
    }

    private var graphChangedHandler: (() -> Void)?
    private var resolutionChangedHandler: (() -> Void)?
    private var libraryUpdateHandler: (() -> Void)?

    // MARK: - Library

    func fetchLibraryFiles() {
        ntw_fetch_library_files()
    }

    func libraryFilesLoaded() -> Bool {
        ntw_library_files_loaded() != 0
    }

    func libraryFiles() -> [LibraryFileInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_library_files(&count), count > 0 else { return [] }
        defer { ntw_free_library_file_info_array(ptr, count) }
        return (0..<Int(count)).map { LibraryFileInfo(from: ptr[$0]) }
    }

    @discardableResult
    func addLibraryVideoSource(libraryFileId: String) -> String? {
        guard let cStr = ntw_add_library_video_source(libraryFileId) else { return nil }
        let result = String(cString: cStr)
        ntw_free_string(cStr)
        return result
    }

    func pauseLibraryDownload(libraryFileId: String) {
        ntw_pause_library_download(libraryFileId)
    }

    func resumeLibraryDownload(libraryFileId: String) {
        ntw_resume_library_download(libraryFileId)
    }

    func registerLibraryUpdateCallback(_ callback: @escaping () -> Void) {
        libraryUpdateHandler = callback
        ntw_register_library_update_callback {
            DispatchQueue.main.async {
                NottawaEngine.shared.libraryUpdateHandler?()
            }
        }
    }

    // MARK: - Library Source State

    func librarySourceState(sourceId: String) -> LibrarySourceState? {
        let s = ntw_get_library_source_state(sourceId)
        defer { if let p = s.libraryFileId { free(UnsafeMutableRawPointer(mutating: p)) } }
        // state 2 = Completed with progress 0 means not a library source
        if s.state == 2 && s.progress == 0 && s.libraryFileId == nil { return nil }
        return LibrarySourceState(
            state: Int(s.state),
            progress: s.progress,
            isPaused: s.isPaused != 0,
            libraryFileId: s.libraryFileId != nil ? String(cString: s.libraryFileId) : nil
        )
    }

    // MARK: - File Source Playback

    func fileSourceState(sourceId: String) -> FileSourceState? {
        let s = ntw_get_file_source_state(sourceId)
        guard s.isFileSource != 0 else { return nil }
        return FileSourceState(
            volume: s.volume,
            speed: s.speed,
            position: s.position,
            duration: s.duration,
            isMuted: s.mute != 0,
            isPlaying: s.playing != 0
        )
    }

    func setFileSourceVolume(sourceId: String, volume: Float) {
        ntw_set_file_source_volume(sourceId, volume)
    }

    func setFileSourceSpeed(sourceId: String, speed: Float) {
        ntw_set_file_source_speed(sourceId, speed)
    }

    func setFileSourcePosition(sourceId: String, position: Float) {
        ntw_set_file_source_position(sourceId, position)
    }

    func setFileSourceMute(sourceId: String, mute: Bool) {
        ntw_set_file_source_mute(sourceId, mute ? 1 : 0)
    }

    func setFileSourcePlaying(sourceId: String, playing: Bool) {
        ntw_set_file_source_playing(sourceId, playing ? 1 : 0)
    }

    // MARK: - Text Source State

    func textSourceState(sourceId: String) -> TextSourceState? {
        let s = ntw_get_text_source_state(sourceId)
        defer {
            if let t = s.text { free(UnsafeMutableRawPointer(mutating: t)) }
            if let names = s.fontNames {
                for i in 0..<Int(s.fontCount) {
                    if let n = names[i] { free(UnsafeMutableRawPointer(mutating: n)) }
                }
                free(UnsafeMutableRawPointer(mutating: names))
            }
        }
        guard s.isTextSource != 0 else { return nil }

        var fontNames: [String] = []
        if let names = s.fontNames {
            for i in 0..<Int(s.fontCount) {
                if let n = names[i] {
                    fontNames.append(String(cString: n))
                }
            }
        }

        return TextSourceState(
            text: s.text != nil ? String(cString: s.text) : "",
            fontSize: Int(s.fontSize),
            xPosition: s.xPosition,
            yPosition: s.yPosition,
            fontIndex: Int(s.fontIndex),
            fontNames: fontNames
        )
    }

    func setTextSourceText(sourceId: String, text: String) {
        ntw_set_text_source_text(sourceId, text)
    }

    func setTextSourceFontSize(sourceId: String, fontSize: Int) {
        ntw_set_text_source_font_size(sourceId, Int32(fontSize))
    }

    func setTextSourceFontIndex(sourceId: String, fontIndex: Int) {
        ntw_set_text_source_font_index(sourceId, Int32(fontIndex))
    }

    func setTextSourcePosition(sourceId: String, x: Float, y: Float) {
        ntw_set_text_source_position(sourceId, x, y)
    }

    // MARK: - Available Non-Shader Sources

    func availableNonShaderSources() -> [AvailableNonShaderSourceInfo] {
        var count: Int32 = 0
        guard let ptr = ntw_get_available_non_shader_sources(&count), count > 0 else { return [] }
        defer { ntw_free_available_non_shader_source_info_array(ptr, count) }
        return (0..<Int(count)).map {
            AvailableNonShaderSourceInfo(
                id: String(cString: ptr[$0].id),
                name: String(cString: ptr[$0].name),
                icon: String(cString: ptr[$0].icon),
                sourceType: Int(ptr[$0].sourceType)
            )
        }
    }

    // MARK: - Texture Sharing

    func startTextureSharing(connectableId: String) {
        ntw_start_texture_sharing(connectableId)
    }

    func stopTextureSharing(connectableId: String) {
        ntw_stop_texture_sharing(connectableId)
    }

    /// Returns the IOSurfaceRef for a shared connectable, or nil if not yet available.
    func connectableIOSurface(connectableId: String) -> IOSurfaceRef? {
        guard let ptr = ntw_get_connectable_texture(connectableId) else { return nil }
        return unsafeBitCast(ptr, to: IOSurfaceRef.self)
    }

    func textureWidth(connectableId: String) -> Int {
        Int(ntw_get_texture_width(connectableId))
    }

    func textureHeight(connectableId: String) -> Int {
        Int(ntw_get_texture_height(connectableId))
    }
}
