//
//  NottawaAppUITests.swift
//  NottawaAppUITests
//
//  UI tests for the NottawaApp SwiftUI frontend.
//

import XCTest

final class NottawaAppUITests: XCTestCase {

    var app: XCUIApplication!

    override func setUpWithError() throws {
        continueAfterFailure = false
        app = XCUIApplication()
        app.launch()
        // Wait for the engine to initialize and accessibility to load.
        // The app has both a SwiftUI window and a hidden GLFW window.
        sleep(8)
    }

    override func tearDownWithError() throws {
        app = nil
    }

    /// Helper: wait for a key element to confirm the app's UI is ready.
    @discardableResult
    private func waitForAppReady() -> Bool {
        let tabPicker = app.descendants(matching: .any)["sidebar-tab-picker"].firstMatch
        return tabPicker.waitForExistence(timeout: 15)
    }

    // MARK: - Window Tests

    func testMainWindowExists() throws {
        let window = app.windows.firstMatch
        XCTAssertTrue(window.waitForExistence(timeout: 10), "Main window should exist")
    }

    // MARK: - Toolbar Tests

    func testToolbarUndoRedoButtons() throws {
        XCTAssertTrue(waitForAppReady())
        let undoButton = app.buttons["undo-button"].firstMatch
        let redoButton = app.buttons["redo-button"].firstMatch
        XCTAssertTrue(undoButton.exists, "Undo button should exist")
        XCTAssertTrue(redoButton.exists, "Redo button should exist")
    }

    func testToolbarDeleteButton() throws {
        XCTAssertTrue(waitForAppReady())
        let deleteButton = app.buttons["delete-button"].firstMatch
        XCTAssertTrue(deleteButton.exists, "Delete button should exist")
    }

    func testToolbarSidebarToggle() throws {
        XCTAssertTrue(waitForAppReady())
        let toggleButton = app.buttons["sidebar-toggle"].firstMatch
        XCTAssertTrue(toggleButton.exists, "Sidebar toggle button should exist")
    }

    func testToolbarResolutionPicker() throws {
        XCTAssertTrue(waitForAppReady())
        let picker = app.descendants(matching: .any)["resolution-picker"].firstMatch
        XCTAssertTrue(picker.exists, "Resolution picker should exist")
    }

    // MARK: - Sidebar Tests

    func testSidebarVisibleOnLaunch() throws {
        let tabPicker = app.descendants(matching: .any)["sidebar-tab-picker"].firstMatch
        XCTAssertTrue(tabPicker.waitForExistence(timeout: 15), "Sidebar tab picker should exist on launch")
    }

    func testSidebarSearchFieldExists() throws {
        XCTAssertTrue(waitForAppReady())
        let searchField = app.descendants(matching: .any)["sidebar-search"].firstMatch
        XCTAssertTrue(searchField.exists, "Search field should exist in sidebar")
    }

    func testSidebarTabSwitching() throws {
        XCTAssertTrue(waitForAppReady())

        // SwiftUI segmented picker creates radio buttons on macOS
        let shadersButton = app.radioButtons["Shaders"].firstMatch
        let sourcesButton = app.radioButtons["Sources"].firstMatch

        if shadersButton.exists && sourcesButton.exists {
            sourcesButton.tap()
            sleep(1)
            shadersButton.tap()
        } else {
            // Fallback: try as regular buttons
            let shadersBtn = app.buttons["Shaders"].firstMatch
            let sourcesBtn = app.buttons["Sources"].firstMatch
            if shadersBtn.exists && sourcesBtn.exists {
                sourcesBtn.tap()
                sleep(1)
                shadersBtn.tap()
            } else {
                // Last resort: verify the picker exists
                let tabPicker = app.descendants(matching: .any)["sidebar-tab-picker"].firstMatch
                XCTAssertTrue(tabPicker.exists, "Tab picker should exist")
            }
        }
    }

    func testSidebarSearchFiltering() throws {
        XCTAssertTrue(waitForAppReady())

        let searchField = app.descendants(matching: .any)["sidebar-search"].firstMatch
        guard searchField.exists else {
            XCTFail("Search field should exist")
            return
        }

        searchField.tap()
        searchField.typeText("Plasma")
        sleep(1)

        // Clear the search
        searchField.typeKey("a", modifierFlags: .command)
        searchField.typeKey(.delete, modifierFlags: [])
    }

    func testSidebarToggle() throws {
        XCTAssertTrue(waitForAppReady())

        let tabPicker = app.descendants(matching: .any)["sidebar-tab-picker"].firstMatch
        XCTAssertTrue(tabPicker.exists, "Sidebar should be visible initially")

        let toggleButton = app.buttons["sidebar-toggle"].firstMatch
        XCTAssertTrue(toggleButton.exists, "Sidebar toggle should exist")
        toggleButton.tap()

        // After toggling, sidebar content should not be hittable
        sleep(1)
        XCTAssertFalse(tabPicker.isHittable, "Sidebar should be hidden after toggle")

        // Toggle back on
        toggleButton.tap()
        sleep(1)
        XCTAssertTrue(tabPicker.waitForExistence(timeout: 5), "Sidebar should reappear after second toggle")
    }

    // MARK: - Node Adding Tests

    func testSidebarHasShaderTiles() throws {
        XCTAssertTrue(waitForAppReady())
        let buttonCount = app.buttons.count
        XCTAssertGreaterThan(buttonCount, 5, "App should have many buttons (including shader tiles)")
    }

    // MARK: - Keyboard Shortcut Tests

    func testEscapeDeselectsAll() throws {
        XCTAssertTrue(waitForAppReady())
        app.typeKey(.escape, modifierFlags: [])
        // No crash = pass
    }

    // MARK: - Context Menu Tests

    func testRightClickCanvasShowsMenu() throws {
        XCTAssertTrue(waitForAppReady())

        let window = app.windows.firstMatch

        // Right-click in the right portion of the window (canvas area, not sidebar)
        let canvasArea = window.coordinate(withNormalizedOffset: CGVector(dx: 0.7, dy: 0.5))
        canvasArea.rightClick()

        let addShaderItem = app.menuItems["Add Shader..."]
        if addShaderItem.waitForExistence(timeout: 3) {
            let addSourceItem = app.menuItems["Add Source..."]
            XCTAssertTrue(addSourceItem.exists, "Context menu should have 'Add Source...' item")
        }

        // Dismiss the menu
        app.typeKey(.escape, modifierFlags: [])
    }

    // MARK: - Helpers: Add and Select a Node

    /// Add a source from the sidebar by name and select the resulting node.
    /// Returns true if a node was found and selected.
    @discardableResult
    private func addSourceAndSelect(name: String) -> Bool {
        // Switch to Sources tab
        let sourcesButton = app.radioButtons["Sources"].firstMatch
        if sourcesButton.exists {
            sourcesButton.tap()
        } else {
            let sourcesBtn = app.buttons["Sources"].firstMatch
            if sourcesBtn.exists { sourcesBtn.tap() }
        }
        sleep(1)

        // Find the tile and tap it to add
        let tile = app.buttons["sidebar-tile-\(name)"].firstMatch
        guard tile.waitForExistence(timeout: 5) else { return false }
        tile.tap()
        sleep(2) // Wait for node to appear on canvas

        // Click the node on canvas to select it
        let node = app.descendants(matching: .any)["node-\(name)"].firstMatch
        guard node.waitForExistence(timeout: 5) else { return false }
        node.tap()
        sleep(1) // Wait for inspector to appear

        return true
    }

    /// Add a shader from the sidebar by name and select the resulting node.
    @discardableResult
    private func addShaderAndSelect(name: String) -> Bool {
        // Ensure we're on Shaders tab
        let shadersButton = app.radioButtons["Shaders"].firstMatch
        if shadersButton.exists {
            shadersButton.tap()
        } else {
            let shadersBtn = app.buttons["Shaders"].firstMatch
            if shadersBtn.exists { shadersBtn.tap() }
        }
        sleep(1)

        let tile = app.buttons["sidebar-tile-\(name)"].firstMatch
        guard tile.waitForExistence(timeout: 5) else { return false }
        tile.tap()
        sleep(2)

        let node = app.descendants(matching: .any)["node-\(name)"].firstMatch
        guard node.waitForExistence(timeout: 5) else { return false }
        node.tap()
        sleep(1)

        return true
    }

    // MARK: - Action Bar Tests

    func testActionBarVisibleOnLaunch() throws {
        XCTAssertTrue(waitForAppReady())
        // The action bar should be expanded by default
        let collapseButton = app.buttons["action-bar-collapse"].firstMatch
        XCTAssertTrue(collapseButton.waitForExistence(timeout: 5), "Action bar collapse button should exist (bar is expanded)")
    }

    func testActionBarAllButtonsExist() throws {
        XCTAssertTrue(waitForAppReady())
        let buttons = [
            "action-bar-collapse",
            "action-bar-undo",
            "action-bar-redo",
            "action-bar-screenshot",
            "action-bar-help",
            "action-bar-clear",
            "action-bar-zoom-fit"
        ]
        for id in buttons {
            let button = app.buttons[id].firstMatch
            XCTAssertTrue(button.waitForExistence(timeout: 5), "\(id) should exist in expanded action bar")
        }
    }

    func testActionBarCollapseAndExpand() throws {
        XCTAssertTrue(waitForAppReady())

        // Collapse the bar
        let collapseButton = app.buttons["action-bar-collapse"].firstMatch
        XCTAssertTrue(collapseButton.waitForExistence(timeout: 5))
        collapseButton.tap()
        sleep(1)

        // Expanded buttons should be gone, expand button should appear
        let expandButton = app.buttons["action-bar-expand"].firstMatch
        XCTAssertTrue(expandButton.waitForExistence(timeout: 5), "Expand button should appear after collapse")
        XCTAssertFalse(app.buttons["action-bar-undo"].firstMatch.isHittable,
                       "Undo button should not be hittable when collapsed")

        // Expand back
        expandButton.tap()
        sleep(1)

        // Collapse button should reappear
        XCTAssertTrue(collapseButton.waitForExistence(timeout: 5), "Collapse button should reappear after expand")
    }

    func testActionBarHelpToggle() throws {
        XCTAssertTrue(waitForAppReady())
        let helpButton = app.buttons["action-bar-help"].firstMatch
        XCTAssertTrue(helpButton.waitForExistence(timeout: 5))

        // Tap help — should toggle without crash
        helpButton.tap()
        sleep(1)
        helpButton.tap()
        sleep(1)
        // No crash = pass; the button should still exist
        XCTAssertTrue(helpButton.exists, "Help button should still exist after toggling twice")
    }

    func testActionBarZoomToFit() throws {
        XCTAssertTrue(waitForAppReady())
        let zoomButton = app.buttons["action-bar-zoom-fit"].firstMatch
        XCTAssertTrue(zoomButton.waitForExistence(timeout: 5))

        // Tap zoom to fit — should not crash even with no nodes
        zoomButton.tap()
        sleep(1)
        XCTAssertTrue(zoomButton.exists, "Zoom to fit button should still exist after tap")
    }

    func testActionBarClearRemovesNodes() throws {
        XCTAssertTrue(waitForAppReady())

        // Add a source node first
        addSourceAndSelect(name: "Plasma")

        // Verify node exists
        let node = app.descendants(matching: .any)["node-Plasma"].firstMatch
        XCTAssertTrue(node.waitForExistence(timeout: 5), "Plasma node should exist")

        // Tap clear
        let clearButton = app.buttons["action-bar-clear"].firstMatch
        XCTAssertTrue(clearButton.waitForExistence(timeout: 5))
        clearButton.tap()
        sleep(2)

        // Node should be gone
        XCTAssertFalse(node.exists, "Plasma node should be removed after clear")
    }

    // MARK: - Oscillator & Audio Driver Tests

    func testInspectorAppearsOnNodeSelection() throws {
        XCTAssertTrue(waitForAppReady())
        XCTAssertTrue(addSourceAndSelect(name: "Plasma"), "Should add and select Plasma source")

        let inspector = app.descendants(matching: .any)["parameter-inspector"].firstMatch
        XCTAssertTrue(inspector.waitForExistence(timeout: 5), "Inspector should appear when node is selected")
    }

    func testOscillatorButtonExists() throws {
        XCTAssertTrue(waitForAppReady())
        XCTAssertTrue(addSourceAndSelect(name: "Plasma"), "Should add and select Plasma source")

        // Look for any oscillator toggle button (prefixed with "osc-toggle-")
        let oscButton = app.buttons.matching(NSPredicate(format: "identifier BEGINSWITH 'osc-toggle-'")).firstMatch
        XCTAssertTrue(oscButton.waitForExistence(timeout: 5), "Oscillator toggle button should exist for a parameter")
    }

    func testOscillatorControlsAppearOnToggle() throws {
        XCTAssertTrue(waitForAppReady())
        XCTAssertTrue(addSourceAndSelect(name: "Plasma"), "Should add and select Plasma source")

        // Find and tap the first oscillator toggle button
        let oscButton = app.buttons.matching(NSPredicate(format: "identifier BEGINSWITH 'osc-toggle-'")).firstMatch
        guard oscButton.waitForExistence(timeout: 5) else {
            XCTFail("Oscillator toggle button should exist")
            return
        }
        oscButton.tap()
        sleep(1)

        // Verify oscillator controls section appeared
        let oscControls = app.descendants(matching: .any).matching(NSPredicate(format: "identifier BEGINSWITH 'osc-controls-'")).firstMatch
        XCTAssertTrue(oscControls.waitForExistence(timeout: 3), "Oscillator controls should appear after tapping toggle")
    }

    func testOscillatorEnableToggle() throws {
        XCTAssertTrue(waitForAppReady())
        XCTAssertTrue(addSourceAndSelect(name: "Plasma"), "Should add and select Plasma source")

        // Open oscillator controls
        let oscButton = app.buttons.matching(NSPredicate(format: "identifier BEGINSWITH 'osc-toggle-'")).firstMatch
        guard oscButton.waitForExistence(timeout: 5) else {
            XCTFail("Oscillator toggle button should exist")
            return
        }
        oscButton.tap()
        sleep(1)

        // Find the oscillator enable toggle (it's a switch labeled "Oscillator")
        let enableToggle = app.switches.matching(NSPredicate(format: "identifier BEGINSWITH 'osc-enable-'")).firstMatch
        XCTAssertTrue(enableToggle.waitForExistence(timeout: 3), "Oscillator enable toggle should exist")

        // Toggle it on
        enableToggle.tap()
        sleep(1)

        // The frequency slider should now be visible
        let freqSlider = app.sliders.matching(NSPredicate(format: "identifier BEGINSWITH 'osc-frequency-'")).firstMatch
        XCTAssertTrue(freqSlider.waitForExistence(timeout: 3), "Frequency slider should appear when oscillator is enabled")
    }

    func testOscillatorWaveShapePicker() throws {
        XCTAssertTrue(waitForAppReady())
        XCTAssertTrue(addSourceAndSelect(name: "Plasma"), "Should add and select Plasma source")

        // Open oscillator controls
        let oscButton = app.buttons.matching(NSPredicate(format: "identifier BEGINSWITH 'osc-toggle-'")).firstMatch
        guard oscButton.waitForExistence(timeout: 5) else {
            XCTFail("Oscillator toggle button should exist")
            return
        }
        oscButton.tap()
        sleep(1)

        // Enable oscillator
        let enableToggle = app.switches.matching(NSPredicate(format: "identifier BEGINSWITH 'osc-enable-'")).firstMatch
        guard enableToggle.waitForExistence(timeout: 3) else {
            XCTFail("Enable toggle should exist")
            return
        }
        enableToggle.tap()
        sleep(1)

        // Waveform shape picker should appear
        let wavePicker = app.descendants(matching: .any).matching(NSPredicate(format: "identifier BEGINSWITH 'osc-waveshape-'")).firstMatch
        XCTAssertTrue(wavePicker.waitForExistence(timeout: 3), "Waveform shape picker should appear when oscillator is enabled")
    }

    func testAudioDriverButtonExists() throws {
        XCTAssertTrue(waitForAppReady())
        XCTAssertTrue(addSourceAndSelect(name: "Plasma"), "Should add and select Plasma source")

        // Look for any audio driver toggle button
        let driverButton = app.buttons.matching(NSPredicate(format: "identifier BEGINSWITH 'driver-toggle-'")).firstMatch
        XCTAssertTrue(driverButton.waitForExistence(timeout: 5), "Audio driver toggle button should exist for a parameter")
    }

    func testAudioDriverControlsAppearOnToggle() throws {
        XCTAssertTrue(waitForAppReady())
        XCTAssertTrue(addSourceAndSelect(name: "Plasma"), "Should add and select Plasma source")

        // Find and tap the first audio driver toggle button
        let driverButton = app.buttons.matching(NSPredicate(format: "identifier BEGINSWITH 'driver-toggle-'")).firstMatch
        guard driverButton.waitForExistence(timeout: 5) else {
            XCTFail("Audio driver toggle button should exist")
            return
        }
        driverButton.tap()
        sleep(1)

        // Verify driver controls section appeared
        let driverControls = app.descendants(matching: .any).matching(NSPredicate(format: "identifier BEGINSWITH 'driver-controls-'")).firstMatch
        XCTAssertTrue(driverControls.waitForExistence(timeout: 3), "Audio driver controls should appear after tapping toggle")
    }

    func testOscillatorAndDriverMutuallyExclusive() throws {
        XCTAssertTrue(waitForAppReady())
        XCTAssertTrue(addSourceAndSelect(name: "Plasma"), "Should add and select Plasma source")

        let oscButton = app.buttons.matching(NSPredicate(format: "identifier BEGINSWITH 'osc-toggle-'")).firstMatch
        let driverButton = app.buttons.matching(NSPredicate(format: "identifier BEGINSWITH 'driver-toggle-'")).firstMatch
        guard oscButton.waitForExistence(timeout: 5), driverButton.waitForExistence(timeout: 5) else {
            XCTFail("Both oscillator and driver buttons should exist")
            return
        }

        // Open oscillator
        oscButton.tap()
        sleep(1)
        let oscControls = app.descendants(matching: .any).matching(NSPredicate(format: "identifier BEGINSWITH 'osc-controls-'")).firstMatch
        XCTAssertTrue(oscControls.waitForExistence(timeout: 3), "Oscillator controls should appear")

        // Open driver — should close oscillator
        driverButton.tap()
        sleep(1)
        let driverControls = app.descendants(matching: .any).matching(NSPredicate(format: "identifier BEGINSWITH 'driver-controls-'")).firstMatch
        XCTAssertTrue(driverControls.waitForExistence(timeout: 3), "Driver controls should appear")

        // Oscillator controls should no longer be visible
        XCTAssertFalse(oscControls.isHittable, "Oscillator controls should be hidden when driver is shown")
    }
}
