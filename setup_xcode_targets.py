#!/usr/bin/env python3
"""
Setup Xcode targets for the Nottawa SwiftUI rebuild.

This script:
1. Adds new C++ Engine/Bridge files to the existing dude_wake_up target
2. Creates a new NottawaApp SwiftUI target that shares the C++ engine code

Usage: python3 setup_xcode_targets.py
"""

import sys
import os

sys.path.insert(0, os.path.expanduser('~/Library/Python/3.13/lib/python/site-packages'))

from pbxproj import XcodeProject, PBXGroup
from pbxproj.pbxsections import *

PROJECT_PATH = 'dude_wake_up.xcodeproj/project.pbxproj'

def add_cpp_files_to_existing_target(project):
    """Add Engine and Bridge C++ files to the existing dude_wake_up target."""
    target = project.objects.get_targets()[0]  # dude_wake_up target
    target_name = target.name

    # New source files to add
    engine_files = [
        ('src/Engine/EngineLoop.cpp', 'sourcecode.cpp.cpp'),
        ('src/Engine/EngineLoop.hpp', 'sourcecode.cpp.h'),
        ('src/Engine/EngineConfig.hpp', 'sourcecode.cpp.h'),
    ]

    bridge_files = [
        ('src/Bridge/nottawa_bridge.cpp', 'sourcecode.cpp.cpp'),
        ('src/Bridge/nottawa_bridge.h', 'sourcecode.c.h'),
    ]

    # Add files to the project
    for filepath, filetype in engine_files + bridge_files:
        flags = project.add_file(filepath, target_name=target_name)
        if flags:
            print(f"  Added {filepath} to {target_name}")
        else:
            print(f"  (already exists or skipped) {filepath}")

    # Add header search paths for the new directories
    for config in project.objects.get_configurations_on_targets():
        bs = config.buildSettings
        existing_paths = bs.get('HEADER_SEARCH_PATHS', '$(OF_CORE_HEADERS)')
        if isinstance(existing_paths, str):
            existing_paths = [existing_paths]

        new_paths = ['src/Engine', 'src/Bridge']
        for p in new_paths:
            if p not in existing_paths:
                existing_paths.append(p)

        bs['HEADER_SEARCH_PATHS'] = existing_paths

    print("  Updated HEADER_SEARCH_PATHS with src/Engine and src/Bridge")


def create_swiftui_target(project):
    """Create a new NottawaApp target for the SwiftUI frontend."""

    # Check if target already exists
    for t in project.objects.get_targets():
        if t.name == 'NottawaApp':
            print("  NottawaApp target already exists, skipping creation")
            return

    # Swift source files
    swift_files = [
        'NottawaApp/NottawaApp.swift',
        'NottawaApp/Engine/BridgeTypes.swift',
        'NottawaApp/Engine/NottawaEngine.swift',
        'NottawaApp/Engine/EngineThread.swift',
        'NottawaApp/Models/GraphModel.swift',
    ]

    # C++ source files shared with the engine
    # We need to compile the bridge + engine + all the existing C++ sources
    cpp_bridge_files = [
        'src/Bridge/nottawa_bridge.cpp',
        'src/Engine/EngineLoop.cpp',
    ]

    target_name = 'NottawaApp'

    # Add Swift files to project
    for filepath in swift_files:
        project.add_file(filepath, target_name=None)  # Add to project but not to any target yet
        print(f"  Added {filepath} to project file references")

    print(f"\n  NottawaApp files added to project file references.")
    print(f"  NOTE: You must manually create the NottawaApp target in Xcode:")
    print(f"    1. File > New > Target > macOS > App")
    print(f"    2. Product Name: NottawaApp")
    print(f"    3. Interface: SwiftUI, Language: Swift")
    print(f"    4. See setup_target_instructions.md for full configuration")


def main():
    if not os.path.exists(PROJECT_PATH):
        print(f"Error: {PROJECT_PATH} not found. Run from project root directory.")
        sys.exit(1)

    print(f"Loading {PROJECT_PATH}...")
    project = XcodeProject.load(PROJECT_PATH)

    print("\n--- Step 1: Add C++ files to existing target ---")
    add_cpp_files_to_existing_target(project)

    print("\n--- Step 2: Add SwiftUI file references ---")
    create_swiftui_target(project)

    print(f"\nSaving {PROJECT_PATH}...")
    project.save()
    print("Done!")


if __name__ == '__main__':
    main()
