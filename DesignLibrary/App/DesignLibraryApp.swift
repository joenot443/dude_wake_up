import SwiftUI

@main
struct DesignLibraryApp: App {
    @State private var themeManager = ThemeManager()

    var body: some Scene {
        WindowGroup {
            ShowcaseView()
                .environment(themeManager)
        }
        .defaultSize(width: 1500, height: 900)
    }
}
