//
//  WorkspaceUITests.swift
//  NottawaAppUITests
//
//  UI tests for workspace save/load/restore workflows including
//  quit-and-relaunch persistence.
//

import XCTest

final class WorkspaceUITests: XCTestCase {

    var app: XCUIApplication!

    override func setUpWithError() throws {
        continueAfterFailure = false

        // Launch the app using the default XCUIApplication (path-based).
        // This triggers the proper XCUITest automation session.
        let launcher = XCUIApplication()
        launcher.launchArguments += ["-nottawa_welcome_enabled", "NO"]
        launcher.launch()
        sleep(15)

        // Re-attach via bundle identifier — this can pick up the SwiftUI
        // accessibility tree that the path-based connection may miss when
        // GLFW creates its own window context.
        app = XCUIApplication(bundleIdentifier: "com.joecrozier.nottawaapp")
        app.activate()
        sleep(3)
    }

    override func tearDownWithError() throws {
        app?.terminate()
        sleep(2)
        app = nil
    }

    // MARK: - Helpers

    @discardableResult
    private func waitForAppReady() -> Bool {
        // Don't check app.windows — GLFW makes windows invisible to XCUITest.
        // Instead wait for the menu bar (always visible) then the sidebar.
        let menuBar = app.menuBars.firstMatch
        guard menuBar.waitForExistence(timeout: 30) else { return false }

        dismissWelcomeScreen()

        let tabPicker = app.descendants(matching: .any)["sidebar-tab-picker"].firstMatch
        if tabPicker.waitForExistence(timeout: 30) { return true }

        dismissWelcomeScreen()
        return tabPicker.waitForExistence(timeout: 15)
    }

    /// Dismiss the welcome screen by pressing Escape.
    /// Does NOT use File > New (which would clear the graph).
    private func dismissWelcomeScreen() {
        // Check if welcome screen is showing (scrim is a Color, so check as any element)
        let scrim = app.descendants(matching: .any)["welcome-scrim"].firstMatch
        guard scrim.waitForExistence(timeout: 3) else { return }
        app.typeKey(.escape, modifierFlags: [])
        sleep(2)
    }

    private func relaunchApp() {
        app.terminate()
        sleep(3)
        app = XCUIApplication()
        app.launchArguments += ["-nottawa_welcome_enabled", "NO"]
        app.launch()
        sleep(20)
    }

    /// Add a shader node by dragging from the sidebar to the canvas.
    @discardableResult
    private func addShader(name: String) -> Bool {
        // Ensure Shaders tab
        let btn = app.buttons["Shaders"].firstMatch
        if btn.waitForExistence(timeout: 5) { btn.tap(); sleep(1) }

        let tile = app.buttons["sidebar-tile-\(name)"].firstMatch
        guard tile.waitForExistence(timeout: 10) else { return false }

        // Drag to canvas area (past the sidebar, right of center)
        let window = app.windows.firstMatch
        let target = window.coordinate(withNormalizedOffset: CGVector(dx: 0.7, dy: 0.5))
        tile.coordinate(withNormalizedOffset: CGVector(dx: 0.5, dy: 0.5))
            .press(forDuration: 1.0, thenDragTo: target)
        sleep(5)

        return app.descendants(matching: .any)["node-\(name)"].firstMatch
            .waitForExistence(timeout: 10)
    }

    private var workspaceNameLabel: XCUIElement {
        app.staticTexts["workspace-name"].firstMatch
    }

    private func tapMenuItem(_ itemName: String) {
        let menuBar = app.menuBars.firstMatch
        guard menuBar.waitForExistence(timeout: 3) else { return }
        menuBar.menuBarItems["File"].tap()
        sleep(1)
        let item = app.menuItems[itemName].firstMatch
        if item.waitForExistence(timeout: 3) { item.tap() }
        sleep(2)
    }

    private func pressSave() { tapMenuItem("Save") }

    private func pressSaveAs() {
        let menuBar = app.menuBars.firstMatch
        guard menuBar.waitForExistence(timeout: 3) else { return }
        menuBar.menuBarItems["File"].tap()
        sleep(1)
        for item in app.menuItems.allElementsBoundByIndex {
            if item.title.hasPrefix("Save As") { item.tap(); sleep(2); return }
        }
        app.typeKey(.escape, modifierFlags: [])
    }

    private func pressOpen() {
        let menuBar = app.menuBars.firstMatch
        guard menuBar.waitForExistence(timeout: 3) else { return }
        menuBar.menuBarItems["File"].tap()
        sleep(1)
        for item in app.menuItems.allElementsBoundByIndex {
            if item.title.hasPrefix("Open") { item.tap(); sleep(2); return }
        }
        app.typeKey(.escape, modifierFlags: [])
    }

    private func pressNew() {
        tapMenuItem("New")
        sleep(1)
    }

    /// Accept the save panel with the default filename by clicking Save in the sheet.
    /// Returns true if the sheet appeared and Save was clicked.
    @discardableResult
    private func acceptSavePanel() -> Bool {
        sleep(2)
        let sheet = app.sheets.firstMatch
        guard sheet.waitForExistence(timeout: 10) else { return false }
        let saveButton = sheet.buttons["Save"].firstMatch
        guard saveButton.waitForExistence(timeout: 5) else { return false }
        saveButton.tap()
        sleep(3)
        // Handle "Replace?" confirmation
        let replace = app.buttons["Replace"].firstMatch
        if replace.waitForExistence(timeout: 2) { replace.tap(); sleep(1) }
        return true
    }

    /// Cancel any open sheet (save/open panel).
    private func cancelSheet() {
        let sheet = app.sheets.firstMatch
        if sheet.waitForExistence(timeout: 3) {
            let cancel = sheet.buttons["Cancel"].firstMatch
            if cancel.exists { cancel.tap() } else { app.typeKey(.escape, modifierFlags: []) }
        } else {
            app.typeKey(.escape, modifierFlags: [])
        }
        sleep(1)
    }

    // MARK: - Tests: File Menu

    func testFileMenuHasSaveItems() throws {
        XCTAssertTrue(waitForAppReady())

        let menuBar = app.menuBars.firstMatch
        menuBar.menuBarItems["File"].tap()
        sleep(1)

        var titles: [String] = []
        for item in app.menuItems.allElementsBoundByIndex {
            if !item.title.isEmpty { titles.append(item.title) }
        }

        XCTAssertTrue(titles.contains("Save"), "File menu should have Save. Found: \(titles)")
        XCTAssertTrue(titles.contains(where: { $0.hasPrefix("Save As") }), "File menu should have Save As")
        XCTAssertTrue(titles.contains("New"), "File menu should have New")
        XCTAssertTrue(titles.contains(where: { $0.hasPrefix("Open") }), "File menu should have Open")

        app.typeKey(.escape, modifierFlags: [])
    }

    // MARK: - Tests: Initial State

    func testUntitledWorkspaceOnFreshLaunch() throws {
        XCTAssertTrue(waitForAppReady())
        pressNew()
        XCTAssertFalse(workspaceNameLabel.exists, "No workspace name for untitled session")
    }

    // MARK: - Tests: Save As

    func testSaveAsShowsSheetAndCreatesWorkspace() throws {
        XCTAssertTrue(waitForAppReady())

        XCTAssertTrue(addShader(name: "Blur"), "Should add Blur shader")

        pressSaveAs()

        // Sheet should appear
        let sheet = app.sheets.firstMatch
        XCTAssertTrue(sheet.waitForExistence(timeout: 10), "Save As should show a sheet")

        // Accept default filename
        XCTAssertTrue(acceptSavePanel(), "Should be able to click Save")

        // Workspace name should now appear in toolbar
        XCTAssertTrue(workspaceNameLabel.waitForExistence(timeout: 5),
                      "Workspace name should appear after Save As")
    }

    // MARK: - Tests: Save (Cmd+S)

    func testCmdSOnUntitledShowsSaveSheet() throws {
        XCTAssertTrue(waitForAppReady())
        pressNew()

        pressSave()

        // A sheet should appear since no workspace is set
        let sheet = app.sheets.firstMatch
        XCTAssertTrue(sheet.waitForExistence(timeout: 10), "Save on untitled should show sheet")

        cancelSheet()
    }

    func testCmdSSavesExistingWorkspace() throws {
        XCTAssertTrue(waitForAppReady())

        XCTAssertTrue(addShader(name: "Blur"), "Should add Blur shader")

        // Save As first to establish workspace
        pressSaveAs()
        XCTAssertTrue(acceptSavePanel(), "Save As should succeed")
        XCTAssertTrue(workspaceNameLabel.waitForExistence(timeout: 5), "Workspace name should appear")

        // Now Cmd+S should NOT show a sheet
        pressSave()
        sleep(2)
        let sheet = app.sheets.firstMatch
        XCTAssertFalse(sheet.exists, "Cmd+S on existing workspace should not show sheet")
    }

    // MARK: - Tests: New Workspace

    func testNewClearsGraphAndWorkspaceName() throws {
        XCTAssertTrue(waitForAppReady())

        // Add content and save
        XCTAssertTrue(addShader(name: "Blur"))
        pressSaveAs()
        XCTAssertTrue(acceptSavePanel())
        XCTAssertTrue(workspaceNameLabel.waitForExistence(timeout: 5), "Name should be set")

        // New workspace
        pressNew()

        // Graph should be empty
        let blurNode = app.descendants(matching: .any)["node-Blur"].firstMatch
        XCTAssertFalse(blurNode.exists, "Nodes should be cleared after New")

        // Workspace name should be gone
        XCTAssertFalse(workspaceNameLabel.exists, "Workspace name should be cleared after New")
    }

    // MARK: - Tests: Open Shows Sheet

    func testOpenShowsSheet() throws {
        XCTAssertTrue(waitForAppReady())

        pressOpen()

        let sheet = app.sheets.firstMatch
        XCTAssertTrue(sheet.waitForExistence(timeout: 10), "Open should show a sheet")

        cancelSheet()
    }

    // MARK: - Tests: Quit and Relaunch Persistence

    func testQuitAndRelaunchRestoresWorkspaceName() throws {
        XCTAssertTrue(waitForAppReady())

        // 1. Add content and save
        XCTAssertTrue(addShader(name: "Blur"))
        pressSaveAs()
        XCTAssertTrue(acceptSavePanel())

        // Remember the workspace name
        XCTAssertTrue(workspaceNameLabel.waitForExistence(timeout: 5))
        let savedName = workspaceNameLabel.label

        // 2. Quit and relaunch
        relaunchApp()
        XCTAssertTrue(waitForAppReady())

        // 3. Workspace name should be restored
        XCTAssertTrue(workspaceNameLabel.waitForExistence(timeout: 10),
                      "Workspace name should be restored after relaunch")
        XCTAssertEqual(workspaceNameLabel.label, savedName,
                       "Workspace name should match what was saved before quit")
    }

    func testQuitAndRelaunchRestoresNodes() throws {
        XCTAssertTrue(waitForAppReady())

        // 1. Add content and save
        XCTAssertTrue(addShader(name: "Blur"))
        pressSaveAs()
        XCTAssertTrue(acceptSavePanel())

        // 2. Quit and relaunch
        relaunchApp()
        XCTAssertTrue(waitForAppReady())

        // 3. Node should be restored
        let blurNode = app.descendants(matching: .any)["node-Blur"].firstMatch
        XCTAssertTrue(blurNode.waitForExistence(timeout: 10),
                      "Blur node should persist after quit and relaunch")
    }

    func testQuitUntitledAndRelaunchRestoresNodes() throws {
        XCTAssertTrue(waitForAppReady())

        // 1. Add content WITHOUT saving as workspace
        XCTAssertTrue(addShader(name: "Blur"))

        // 2. Wait for auto-save, then quit
        sleep(5)
        relaunchApp()
        XCTAssertTrue(waitForAppReady())

        // 3. Auto-saved config should restore the node
        let blurNode = app.descendants(matching: .any)["node-Blur"].firstMatch
        XCTAssertTrue(blurNode.waitForExistence(timeout: 10),
                      "Nodes should auto-save and restore even without a workspace")

        // 4. But no workspace name should show
        XCTAssertFalse(workspaceNameLabel.exists,
                       "No workspace name for auto-saved untitled session")
    }

    // MARK: - Tests: Save Overwrites

    func testSaveOverwritesWithoutShowingSheet() throws {
        XCTAssertTrue(waitForAppReady())

        // 1. Create workspace
        XCTAssertTrue(addShader(name: "Blur"))
        pressSaveAs()
        XCTAssertTrue(acceptSavePanel())

        // 2. Add more content
        XCTAssertTrue(addShader(name: "Blur"))
        sleep(1)

        // 3. Cmd+S should save silently (no sheet)
        pressSave()
        sleep(3)
        XCTAssertFalse(app.sheets.firstMatch.exists,
                       "Save on existing workspace should not show sheet")
    }

    // MARK: - Tests: Workspace Indicator

    func testWorkspaceNameAppearsAfterSaveAs() throws {
        XCTAssertTrue(waitForAppReady())

        // Initially no workspace name
        pressNew()
        XCTAssertFalse(workspaceNameLabel.exists, "No name before save")

        // Add content and save
        XCTAssertTrue(addShader(name: "Blur"))
        pressSaveAs()
        XCTAssertTrue(acceptSavePanel())

        // Name should now appear
        XCTAssertTrue(workspaceNameLabel.waitForExistence(timeout: 5),
                      "Workspace name should appear after Save As")
    }

    func testWorkspaceNameClearedByNew() throws {
        XCTAssertTrue(waitForAppReady())

        // Save to establish workspace
        XCTAssertTrue(addShader(name: "Blur"))
        pressSaveAs()
        XCTAssertTrue(acceptSavePanel())
        XCTAssertTrue(workspaceNameLabel.waitForExistence(timeout: 5))

        // New should clear it
        pressNew()
        XCTAssertFalse(workspaceNameLabel.exists, "Name should be gone after New")
    }
}
