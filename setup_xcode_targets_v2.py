#!/usr/bin/env python3
"""
Setup Xcode targets for the Nottawa SwiftUI rebuild.

This script directly manipulates the project.pbxproj text to:
1. Add Engine/Bridge C++ files to the existing dude_wake_up target
2. Create a new NottawaApp SwiftUI target that shares all C++ engine code

This avoids the pbxproj Python library which corrupts file references.
"""

import re
import uuid
import sys
import os
import shutil

PROJECT_PATH = 'dude_wake_up.xcodeproj/project.pbxproj'

def gen_uuid():
    """Generate a 24-char hex UUID like Xcode uses."""
    return uuid.uuid4().hex[:24].upper()

def read_project():
    with open(PROJECT_PATH, 'r') as f:
        return f.read()

def write_project(content):
    with open(PROJECT_PATH, 'w') as f:
        f.write(content)

def backup_project():
    backup = PROJECT_PATH + '.bak2'
    shutil.copy2(PROJECT_PATH, backup)
    print(f"Backed up to {backup}")

# ---------------------------------------------------------------------------
# Step 1: Collect existing source file refs from the Sources build phase
# ---------------------------------------------------------------------------

def get_existing_source_build_files(content):
    """Parse the existing Sources build phase to get all build file IDs."""
    # Find the Sources build phase
    pattern = r'E4B69B580A3A1756003C02F2 /\* Sources \*/ = \{[^}]*files = \((.*?)\);'
    match = re.search(pattern, content, re.DOTALL)
    if not match:
        print("ERROR: Could not find Sources build phase")
        sys.exit(1)

    files_block = match.group(1)
    # Extract each build file ID
    bf_ids = re.findall(r'([A-F0-9]{24}|"[A-F0-9-]+")', files_block)
    # Strip quotes
    bf_ids = [bid.strip('"') for bid in bf_ids]
    return bf_ids

def get_build_file_to_file_ref_mapping(content):
    """Map build file IDs to their file reference IDs."""
    mapping = {}
    # Match: ID /* name in phase */ = {isa = PBXBuildFile; fileRef = REFID /* name */; };
    pattern = r'(?:"([A-F0-9-]+)"|([A-F0-9]{24}))\s*/\*.*?\*/\s*=\s*\{isa = PBXBuildFile;\s*fileRef\s*=\s*(?:"([A-F0-9-]+)"|([A-F0-9]{24}))\s*/\*.*?\*/\s*;\s*\};'
    for m in re.finditer(pattern, content):
        bf_id = m.group(1) or m.group(2)
        fr_id = m.group(3) or m.group(4)
        mapping[bf_id] = fr_id
    return mapping

def get_file_ref_paths(content):
    """Map file reference IDs to their paths."""
    mapping = {}
    # Match PBXFileReference entries
    pattern = r'(?:"([A-F0-9-]+)"|([A-F0-9]{24}))\s*/\*\s*(.*?)\s*\*/\s*=\s*\{isa = PBXFileReference;.*?(?:path\s*=\s*(?:"([^"]+)"|(\S+?));|name\s*=\s*(?:"([^"]+)"|(\S+?));)'
    for m in re.finditer(pattern, content):
        fr_id = m.group(1) or m.group(2)
        path = m.group(4) or m.group(5) or m.group(6) or m.group(7)
        if path:
            mapping[fr_id] = path
    return mapping

# ---------------------------------------------------------------------------
# Step 2: Define new files to add
# ---------------------------------------------------------------------------

# New C++ files for the engine/bridge
NEW_CPP_FILES = [
    {
        'path': 'src/Engine/EngineLoop.cpp',
        'name': 'EngineLoop.cpp',
        'type': 'sourcecode.cpp.cpp',
    },
    {
        'path': 'src/Engine/EngineLoop.hpp',
        'name': 'EngineLoop.hpp',
        'type': 'sourcecode.cpp.h',
    },
    {
        'path': 'src/Engine/EngineConfig.hpp',
        'name': 'EngineConfig.hpp',
        'type': 'sourcecode.cpp.h',
    },
    {
        'path': 'src/Bridge/nottawa_bridge.cpp',
        'name': 'nottawa_bridge.cpp',
        'type': 'sourcecode.cpp.cpp',
    },
    {
        'path': 'src/Bridge/nottawa_bridge.h',
        'name': 'nottawa_bridge.h',
        'type': 'sourcecode.c.h',
    },
]

# Swift files for NottawaApp
SWIFT_FILES = [
    {
        'path': 'NottawaApp/NottawaApp.swift',
        'name': 'NottawaApp.swift',
        'type': 'sourcecode.swift',
    },
    {
        'path': 'NottawaApp/Engine/BridgeTypes.swift',
        'name': 'BridgeTypes.swift',
        'type': 'sourcecode.swift',
    },
    {
        'path': 'NottawaApp/Engine/NottawaEngine.swift',
        'name': 'NottawaEngine.swift',
        'type': 'sourcecode.swift',
    },
    {
        'path': 'NottawaApp/Engine/EngineThread.swift',
        'name': 'EngineThread.swift',
        'type': 'sourcecode.swift',
    },
    {
        'path': 'NottawaApp/Models/GraphModel.swift',
        'name': 'GraphModel.swift',
        'type': 'sourcecode.swift',
    },
]

BRIDGING_HEADER = {
    'path': 'NottawaApp/NottawaApp-Bridging-Header.h',
    'name': 'NottawaApp-Bridging-Header.h',
    'type': 'sourcecode.c.h',
}


def check_file_exists_in_project(content, path):
    """Check if a file path is already referenced in the project."""
    # Escape path for regex
    escaped = re.escape(path)
    return bool(re.search(rf'path\s*=\s*"{escaped}"|path\s*=\s*{escaped}', content))


def main():
    if not os.path.exists(PROJECT_PATH):
        print(f"Error: {PROJECT_PATH} not found. Run from project root.")
        sys.exit(1)

    backup_project()
    content = read_project()

    # Track all new IDs
    new_file_refs = {}        # path -> file_ref_id
    new_build_files = {}      # path -> build_file_id (for existing target)
    new_build_files_ntw = {}  # path -> build_file_id (for NottawaApp target)

    # -----------------------------------------------------------------------
    # Generate UUIDs for all new objects
    # -----------------------------------------------------------------------

    # Engine/Bridge file refs + build files for existing target
    for f in NEW_CPP_FILES:
        if not check_file_exists_in_project(content, f['path']):
            new_file_refs[f['path']] = gen_uuid()
            # Only .cpp/.c/.mm/.m files get build file entries
            if f['type'] in ('sourcecode.cpp.cpp', 'sourcecode.c.c', 'sourcecode.cpp.objcpp', 'sourcecode.c.objc'):
                new_build_files[f['path']] = gen_uuid()
        else:
            print(f"  (already in project) {f['path']}")

    # Swift file refs + build files for NottawaApp target
    for f in SWIFT_FILES + [BRIDGING_HEADER]:
        if not check_file_exists_in_project(content, f['path']):
            new_file_refs[f['path']] = gen_uuid()
        else:
            print(f"  (already in project) {f['path']}")

    # NottawaApp target IDs
    ntw_target_id = gen_uuid()
    ntw_sources_phase_id = gen_uuid()
    ntw_frameworks_phase_id = gen_uuid()
    ntw_resources_phase_id = gen_uuid()
    ntw_compile_of_phase_id = gen_uuid()
    ntw_config_list_id = gen_uuid()
    ntw_debug_config_id = gen_uuid()
    ntw_release_config_id = gen_uuid()
    ntw_product_ref_id = gen_uuid()
    ntw_headers_phase_id = gen_uuid()

    # Build file IDs for NottawaApp sources phase
    # We need build file entries for ALL existing C++ sources + new C++ sources + Swift files

    # Get existing source build files
    existing_bf_ids = get_existing_source_build_files(content)
    bf_to_fr = get_build_file_to_file_ref_mapping(content)
    fr_paths = get_file_ref_paths(content)

    # For NottawaApp, we need new PBXBuildFile entries pointing to the same file refs
    ntw_source_build_files = []  # (new_bf_id, file_ref_id, name)

    # Copy all existing C++ sources (except main.cpp which has its own main())
    for bf_id in existing_bf_ids:
        if bf_id in bf_to_fr:
            fr_id = bf_to_fr[bf_id]
            path = fr_paths.get(fr_id, '')
            name = os.path.basename(path) if path else '??'
            # Skip main.cpp - NottawaApp has its own @main in Swift
            if name == 'main.cpp':
                continue
            new_bf_id = gen_uuid()
            ntw_source_build_files.append((new_bf_id, fr_id, name))

    # Add new Engine/Bridge .cpp files to NottawaApp
    for f in NEW_CPP_FILES:
        if f['type'] in ('sourcecode.cpp.cpp', 'sourcecode.c.c', 'sourcecode.cpp.objcpp', 'sourcecode.c.objc'):
            fr_id = new_file_refs.get(f['path'])
            if fr_id:
                new_bf_id = gen_uuid()
                ntw_source_build_files.append((new_bf_id, fr_id, f['name']))

    # Add Swift files to NottawaApp
    for f in SWIFT_FILES:
        fr_id = new_file_refs.get(f['path'])
        if fr_id:
            new_bf_id = gen_uuid()
            ntw_source_build_files.append((new_bf_id, fr_id, f['name']))

    # Build file IDs for NottawaApp frameworks
    ntw_sparkle_bf_id = gen_uuid()
    ntw_coremidi_bf_id = gen_uuid()

    # Existing framework file ref IDs
    sparkle_fr_id = '02D080342EE857C3003AD114'
    coremidi_fr_id = '04CA78A1-FF25-425E-B2D1-041597750E58'

    print(f"\nCreating {len(ntw_source_build_files)} source build files for NottawaApp target")

    # -----------------------------------------------------------------------
    # Insert new PBXFileReference entries
    # -----------------------------------------------------------------------

    file_ref_lines = []
    for f in NEW_CPP_FILES + SWIFT_FILES + [BRIDGING_HEADER]:
        fr_id = new_file_refs.get(f['path'])
        if fr_id:
            file_ref_lines.append(
                f'\t\t{fr_id} /* {f["name"]} */ = '
                f'{{isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = {f["type"]}; '
                f'name = "{f["name"]}"; path = "{f["path"]}"; sourceTree = "<group>"; }};'
            )

    # Also add the NottawaApp product reference
    file_ref_lines.append(
        f'\t\t{ntw_product_ref_id} /* NottawaApp.app */ = '
        f'{{isa = PBXFileReference; explicitFileType = wrapper.application; '
        f'includeInIndex = 0; path = NottawaApp.app; sourceTree = BUILT_PRODUCTS_DIR; }};'
    )

    if file_ref_lines:
        # Insert before /* End PBXFileReference section */
        file_ref_block = '\n'.join(file_ref_lines) + '\n'
        content = content.replace(
            '/* End PBXFileReference section */',
            file_ref_block + '/* End PBXFileReference section */'
        )
        print(f"  Added {len(file_ref_lines)} PBXFileReference entries")

    # -----------------------------------------------------------------------
    # Insert new PBXBuildFile entries
    # -----------------------------------------------------------------------

    build_file_lines = []

    # Build files for existing target (new C++ sources)
    for f in NEW_CPP_FILES:
        bf_id = new_build_files.get(f['path'])
        fr_id = new_file_refs.get(f['path'])
        if bf_id and fr_id:
            build_file_lines.append(
                f'\t\t{bf_id} /* {f["name"]} in Sources */ = '
                f'{{isa = PBXBuildFile; fileRef = {fr_id} /* {f["name"]} */; }};'
            )

    # Build files for NottawaApp target
    for bf_id, fr_id, name in ntw_source_build_files:
        # Handle quoted UUIDs (the ones with dashes)
        fr_ref = f'"{fr_id}"' if '-' in fr_id else fr_id
        build_file_lines.append(
            f'\t\t{bf_id} /* {name} in Sources */ = '
            f'{{isa = PBXBuildFile; fileRef = {fr_ref} /* {name} */; }};'
        )

    # Framework build files for NottawaApp
    build_file_lines.append(
        f'\t\t{ntw_sparkle_bf_id} /* Sparkle.framework in Frameworks */ = '
        f'{{isa = PBXBuildFile; fileRef = {sparkle_fr_id} /* Sparkle.framework */; }};'
    )
    build_file_lines.append(
        f'\t\t{ntw_coremidi_bf_id} /* CoreMIDI.framework in Frameworks */ = '
        f'{{isa = PBXBuildFile; fileRef = "{coremidi_fr_id}" /* CoreMIDI.framework */; }};'
    )

    if build_file_lines:
        build_file_block = '\n'.join(build_file_lines) + '\n'
        content = content.replace(
            '/* End PBXBuildFile section */',
            build_file_block + '/* End PBXBuildFile section */'
        )
        print(f"  Added {len(build_file_lines)} PBXBuildFile entries")

    # -----------------------------------------------------------------------
    # Add new .cpp build files to existing target's Sources phase
    # -----------------------------------------------------------------------

    new_source_refs = []
    for f in NEW_CPP_FILES:
        bf_id = new_build_files.get(f['path'])
        if bf_id:
            new_source_refs.append(f'\t\t\t\t{bf_id} /* {f["name"]} in Sources */,')

    if new_source_refs:
        # Find the end of the existing sources phase files list
        # Look for the closing ); of the files = ( block in the Sources phase
        pattern = r'(E4B69B580A3A1756003C02F2 /\* Sources \*/ = \{[^}]*files = \([^)]*)'
        match = re.search(pattern, content, re.DOTALL)
        if match:
            insert_point = match.end()
            insert_text = '\n' + '\n'.join(new_source_refs)
            content = content[:insert_point] + insert_text + content[insert_point:]
            print(f"  Added {len(new_source_refs)} files to existing Sources phase")

    # -----------------------------------------------------------------------
    # Add src/Engine and src/Bridge to existing target's header search paths
    # -----------------------------------------------------------------------

    # Find the Debug config for the target and add header paths
    for config_name in ['Debug', 'Release']:
        # Find the target build config that has the full HEADER_SEARCH_PATHS
        # We need to add src/Engine and src/Bridge
        pattern = rf'(HEADER_SEARCH_PATHS = \([^)]*lib/ableton/asio,)'
        replacement = r'\1\n\t\t\t\t\tsrc/Engine,\n\t\t\t\t\tsrc/Bridge,'
        content, count = re.subn(pattern, replacement, content)
        if count > 0:
            break

    # There are two target configs that need updating - do it for both
    # The pattern above may have only matched once; let me do a more targeted approach
    content = content.replace(
        'lib/ableton/asio,\n\t\t\t\t);',
        'lib/ableton/asio,\n\t\t\t\t\tsrc/Engine,\n\t\t\t\t\tsrc/Bridge,\n\t\t\t\t);'
    )
    print("  Added src/Engine, src/Bridge to HEADER_SEARCH_PATHS")

    # -----------------------------------------------------------------------
    # Create NottawaApp build phases
    # -----------------------------------------------------------------------

    # Sources phase for NottawaApp
    ntw_source_file_entries = []
    for bf_id, fr_id, name in ntw_source_build_files:
        ntw_source_file_entries.append(f'\t\t\t\t{bf_id} /* {name} in Sources */,')

    ntw_sources_phase = f'''\t\t{ntw_sources_phase_id} /* Sources */ = {{
\t\t\tisa = PBXSourcesBuildPhase;
\t\t\tbuildActionMask = 2147483647;
\t\t\tfiles = (
{chr(10).join(ntw_source_file_entries)}
\t\t\t);
\t\t\trunOnlyForDeploymentPostprocessing = 0;
\t\t}};'''

    content = content.replace(
        '/* End PBXSourcesBuildPhase section */',
        ntw_sources_phase + '\n/* End PBXSourcesBuildPhase section */'
    )

    # Frameworks phase for NottawaApp
    ntw_frameworks_phase = f'''\t\t{ntw_frameworks_phase_id} /* Frameworks */ = {{
\t\t\tisa = PBXFrameworksBuildPhase;
\t\t\tbuildActionMask = 2147483647;
\t\t\tfiles = (
\t\t\t\t{ntw_sparkle_bf_id} /* Sparkle.framework in Frameworks */,
\t\t\t\t{ntw_coremidi_bf_id} /* CoreMIDI.framework in Frameworks */,
\t\t\t);
\t\t\trunOnlyForDeploymentPostprocessing = 0;
\t\t}};'''

    content = content.replace(
        '/* End PBXFrameworksBuildPhase section */',
        ntw_frameworks_phase + '\n/* End PBXFrameworksBuildPhase section */'
    )

    # Resources phase for NottawaApp
    ntw_resources_phase = f'''\t\t{ntw_resources_phase_id} /* Resources */ = {{
\t\t\tisa = PBXResourcesBuildPhase;
\t\t\tbuildActionMask = 2147483647;
\t\t\tfiles = (
\t\t\t);
\t\t\trunOnlyForDeploymentPostprocessing = 0;
\t\t}};'''

    content = content.replace(
        '/* End PBXResourcesBuildPhase section */',
        ntw_resources_phase + '\n/* End PBXResourcesBuildPhase section */'
    )

    # Run Script phase: Compile OF for NottawaApp
    ntw_compile_of = f'''\t\t{ntw_compile_of_phase_id} /* Run Script — Compile OF */ = {{
\t\t\tisa = PBXShellScriptBuildPhase;
\t\t\talwaysOutOfDate = 1;
\t\t\tbuildActionMask = 2147483647;
\t\t\tfiles = (
\t\t\t);
\t\t\tinputPaths = (
\t\t\t);
\t\t\tname = "Run Script — Compile OF";
\t\t\toutputPaths = (
\t\t\t);
\t\t\trunOnlyForDeploymentPostprocessing = 0;
\t\t\tshellPath = /bin/sh;
\t\t\tshellScript = "$OF_CORE_BUILD_COMMAND\\n";
\t\t\tshowEnvVarsInLog = 0;
\t\t}};'''

    # Insert before /* End PBXShellScriptBuildPhase section */
    content = content.replace(
        '/* End PBXShellScriptBuildPhase section */',
        ntw_compile_of + '\n/* End PBXShellScriptBuildPhase section */'
    )

    # -----------------------------------------------------------------------
    # Create NottawaApp target
    # -----------------------------------------------------------------------

    ntw_target = f'''\t\t{ntw_target_id} /* NottawaApp */ = {{
\t\t\tisa = PBXNativeTarget;
\t\t\tbuildConfigurationList = {ntw_config_list_id} /* Build configuration list for PBXNativeTarget "NottawaApp" */;
\t\t\tbuildPhases = (
\t\t\t\t{ntw_compile_of_phase_id} /* Run Script — Compile OF */,
\t\t\t\t{ntw_sources_phase_id} /* Sources */,
\t\t\t\t{ntw_frameworks_phase_id} /* Frameworks */,
\t\t\t\t{ntw_resources_phase_id} /* Resources */,
\t\t\t);
\t\t\tbuildRules = (
\t\t\t);
\t\t\tdependencies = (
\t\t\t);
\t\t\tname = NottawaApp;
\t\t\tproductName = NottawaApp;
\t\t\tproductReference = {ntw_product_ref_id} /* NottawaApp.app */;
\t\t\tproductType = "com.apple.product-type.application";
\t\t}};'''

    content = content.replace(
        '/* End PBXNativeTarget section */',
        ntw_target + '\n/* End PBXNativeTarget section */'
    )

    # -----------------------------------------------------------------------
    # Add NottawaApp to project targets list
    # -----------------------------------------------------------------------

    content = content.replace(
        'targets = (\n\t\t\t\tE4B69B5A0A3A1756003C02F2 /* dude_wake_up */,\n\t\t\t);',
        f'targets = (\n\t\t\t\tE4B69B5A0A3A1756003C02F2 /* dude_wake_up */,\n\t\t\t\t{ntw_target_id} /* NottawaApp */,\n\t\t\t);'
    )

    # -----------------------------------------------------------------------
    # Create build configurations for NottawaApp
    # -----------------------------------------------------------------------

    # The bridging header path relative to project root
    bridging_header_ref = new_file_refs.get(BRIDGING_HEADER['path'], '')

    # Common header search paths (same as existing target + Engine/Bridge)
    header_search_paths = '''(
\t\t\t\t\t"$(OF_CORE_HEADERS)",
\t\t\t\t\tsrc,
\t\t\t\t\tsrc/Apps,
\t\t\t\t\tsrc/Audio,
\t\t\t\t\tsrc/Feedback,
\t\t\t\t\tsrc/Models,
\t\t\t\t\tsrc/Network,
\t\t\t\t\tsrc/NodeLayout,
\t\t\t\t\tsrc/Oscillation,
\t\t\t\t\tsrc/Services,
\t\t\t\t\tsrc/Services/Commands,
\t\t\t\t\tsrc/Services/Midi,
\t\t\t\t\tsrc/Shading,
\t\t\t\t\tsrc/Util,
\t\t\t\t\tsrc/Video,
\t\t\t\t\tsrc/Views,
\t\t\t\t\tsrc/Engine,
\t\t\t\t\tsrc/Bridge,
\t\t\t\t\t../../../addons/ofxImGui/libs,
\t\t\t\t\t../../../addons/ofxImGui/libs/imgui,
\t\t\t\t\t../../../addons/ofxImGui/libs/imgui/backends,
\t\t\t\t\t../../../addons/ofxImGui/libs/imgui/docs,
\t\t\t\t\t../../../addons/ofxImGui/libs/imgui/extras,
\t\t\t\t\t../../../addons/ofxImGui/libs/imgui/src,
\t\t\t\t\t../../../addons/ofxImGui/src,
\t\t\t\t\t../../../addons/ofxMidi/libs,
\t\t\t\t\t../../../addons/ofxMidi/libs/pgmidi,
\t\t\t\t\t../../../addons/ofxMidi/libs/rtmidi,
\t\t\t\t\t../../../addons/ofxMidi/src,
\t\t\t\t\t../../../addons/ofxMidi/src/desktop,
\t\t\t\t\t../../../addons/ofxMidi/src/ios,
\t\t\t\t\t../../../addons/ofxSyphon/libs,
\t\t\t\t\t../../../addons/ofxSyphon/src,
\t\t\t\t\t"lib/imgui-node-editor",
\t\t\t\t\t"lib/observable/**",
\t\t\t\t\t"lib/gist/**",
\t\t\t\t\t"lib/implot/**",
\t\t\t\t\t"lib/ableton/**",
\t\t\t\t\tlib/ableton/asio,
\t\t\t\t)'''

    ntw_debug_config = f'''\t\t{ntw_debug_config_id} /* Debug */ = {{
\t\t\tisa = XCBuildConfiguration;
\t\t\tbaseConfigurationReference = E4EB6923138AFD0F00A09F29 /* Project.xcconfig */;
\t\t\tbuildSettings = {{
\t\t\t\tARCHS = "$(ARCHS_STANDARD)";
\t\t\t\tCC = "$(OF_PATH)/scripts/osx/cxx.sh";
\t\t\t\tCLANG_ENABLE_MODULES = YES;
\t\t\t\tCODE_SIGN_IDENTITY = "Apple Development";
\t\t\t\tCODE_SIGN_STYLE = Automatic;
\t\t\t\tCOPY_PHASE_STRIP = NO;
\t\t\t\tCURRENT_PROJECT_VERSION = 1;
\t\t\t\tDEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";
\t\t\t\tDEVELOPMENT_TEAM = 7VUCELU5PL;
\t\t\t\tENABLE_HARDENED_RUNTIME = NO;
\t\t\t\tFRAMEWORK_SEARCH_PATHS = (
\t\t\t\t\t"$(inherited)",
\t\t\t\t\t"$(PROJECT_DIR)/lib",
\t\t\t\t);
\t\t\t\tGCC_DYNAMIC_NO_PIC = YES;
\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = (
\t\t\t\t\t"$(inherited)",
\t\t\t\t\t"$(USER_PREPROCESSOR_DEFINITIONS)",
\t\t\t\t\t"CPPHTTPLIB_OPENSSL_SUPPORT=1",
\t\t\t\t\t"LINK_PLATFORM_MACOSX=1",
\t\t\t\t\t"NOTTAWA_ENGINE_ONLY=1",
\t\t\t\t);
\t\t\t\tHEADER_SEARCH_PATHS = {header_search_paths};
\t\t\t\tINFOPLIST_KEY_CFBundleDisplayName = NottawaApp;
\t\t\t\tINFOPLIST_KEY_NSCameraUsageDescription = "NottawaApp can use your camera as a video source";
\t\t\t\tINFOPLIST_KEY_NSMicrophoneUsageDescription = "NottawaApp can use your microphone for live music analysis";
\t\t\t\tLD_RUNPATH_SEARCH_PATHS = (
\t\t\t\t\t"$(LD_RUNPATH_SEARCH_PATHS_$(IS_MACCATALYST))",
\t\t\t\t\t"@executable_path/../Frameworks",
\t\t\t\t\t"@loader_path/../Frameworks",
\t\t\t\t);
\t\t\t\tLIBRARY_SEARCH_PATHS = "$(inherited)";
\t\t\t\tMACOSX_DEPLOYMENT_TARGET = 14.0;
\t\t\t\tMARKETING_VERSION = 1.0.0;
\t\t\t\tOTHER_CFLAGS = "-DofxAddons_ENABLE_IMGUI";
\t\t\t\tOTHER_CPLUSPLUSFLAGS = "-D__MACOSX_CORE__";
\t\t\t\tOTHER_LDFLAGS = (
\t\t\t\t\t"$(OF_CORE_LIBS)",
\t\t\t\t\t"$(OF_CORE_FRAMEWORKS)",
\t\t\t\t\t"$(LIB_OF_DEBUG)",
\t\t\t\t);
\t\t\t\tPRODUCT_BUNDLE_IDENTIFIER = com.joecrozier.nottawaapp;
\t\t\t\tPRODUCT_NAME = "$(TARGET_NAME)";
\t\t\t\tSWIFT_OBJC_BRIDGING_HEADER = "NottawaApp/NottawaApp-Bridging-Header.h";
\t\t\t\tSWIFT_OPTIMIZATION_LEVEL = "-Onone";
\t\t\t\tSWIFT_VERSION = 5.0;
\t\t\t}};
\t\t\tname = Debug;
\t\t}};'''

    ntw_release_config = f'''\t\t{ntw_release_config_id} /* Release */ = {{
\t\t\tisa = XCBuildConfiguration;
\t\t\tbaseConfigurationReference = E4EB6923138AFD0F00A09F29 /* Project.xcconfig */;
\t\t\tbuildSettings = {{
\t\t\t\tARCHS = "$(ARCHS_STANDARD)";
\t\t\t\tCC = "$(OF_PATH)/scripts/osx/cxx.sh";
\t\t\t\tCLANG_ENABLE_MODULES = YES;
\t\t\t\tCODE_SIGN_IDENTITY = "Apple Development";
\t\t\t\t"CODE_SIGN_IDENTITY[sdk=macosx*]" = "Developer ID Application";
\t\t\t\tCODE_SIGN_STYLE = Manual;
\t\t\t\tCOPY_PHASE_STRIP = NO;
\t\t\t\tCURRENT_PROJECT_VERSION = 1;
\t\t\t\tDEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";
\t\t\t\tDEPLOYMENT_POSTPROCESSING = YES;
\t\t\t\tDEVELOPMENT_TEAM = "";
\t\t\t\t"DEVELOPMENT_TEAM[sdk=macosx*]" = 7VUCELU5PL;
\t\t\t\tENABLE_HARDENED_RUNTIME = YES;
\t\t\t\tFRAMEWORK_SEARCH_PATHS = (
\t\t\t\t\t"$(inherited)",
\t\t\t\t\t"$(PROJECT_DIR)/lib",
\t\t\t\t);
\t\t\t\tGCC_DYNAMIC_NO_PIC = YES;
\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = (
\t\t\t\t\t"$(inherited)",
\t\t\t\t\t"$(USER_PREPROCESSOR_DEFINITIONS)",
\t\t\t\t\t"CPPHTTPLIB_OPENSSL_SUPPORT=1",
\t\t\t\t\t"LINK_PLATFORM_MACOSX=1",
\t\t\t\t\t"NOTTAWA_ENGINE_ONLY=1",
\t\t\t\t);
\t\t\t\tHEADER_SEARCH_PATHS = {header_search_paths};
\t\t\t\tINFOPLIST_KEY_CFBundleDisplayName = NottawaApp;
\t\t\t\tINFOPLIST_KEY_NSCameraUsageDescription = "NottawaApp can use your camera as a video source";
\t\t\t\tINFOPLIST_KEY_NSMicrophoneUsageDescription = "NottawaApp can use your microphone for live music analysis";
\t\t\t\tLD_RUNPATH_SEARCH_PATHS = (
\t\t\t\t\t"$(LD_RUNPATH_SEARCH_PATHS_$(IS_MACCATALYST))",
\t\t\t\t\t"@executable_path/../Frameworks",
\t\t\t\t\t"@loader_path/../Frameworks",
\t\t\t\t);
\t\t\t\tLIBRARY_SEARCH_PATHS = "$(inherited)";
\t\t\t\tMACOSX_DEPLOYMENT_TARGET = 14.0;
\t\t\t\tMARKETING_VERSION = 1.0.0;
\t\t\t\tOTHER_CFLAGS = "-DofxAddons_ENABLE_IMGUI";
\t\t\t\tOTHER_CPLUSPLUSFLAGS = "-D__MACOSX_CORE__";
\t\t\t\tOTHER_LDFLAGS = (
\t\t\t\t\t"$(OF_CORE_LIBS)",
\t\t\t\t\t"$(OF_CORE_FRAMEWORKS)",
\t\t\t\t\t"$(LIB_OF_RELEASE)",
\t\t\t\t);
\t\t\t\tPRODUCT_BUNDLE_IDENTIFIER = com.joecrozier.nottawaapp;
\t\t\t\tPRODUCT_NAME = "$(TARGET_NAME)";
\t\t\t\tRELEASE = 1;
\t\t\t\tSWIFT_OBJC_BRIDGING_HEADER = "NottawaApp/NottawaApp-Bridging-Header.h";
\t\t\t\tSWIFT_VERSION = 5.0;
\t\t\t}};
\t\t\tname = Release;
\t\t}};'''

    content = content.replace(
        '/* End XCBuildConfiguration section */',
        ntw_debug_config + '\n' + ntw_release_config + '\n/* End XCBuildConfiguration section */'
    )

    # -----------------------------------------------------------------------
    # Create configuration list for NottawaApp
    # -----------------------------------------------------------------------

    ntw_config_list = f'''\t\t{ntw_config_list_id} /* Build configuration list for PBXNativeTarget "NottawaApp" */ = {{
\t\t\tisa = XCConfigurationList;
\t\t\tbuildConfigurations = (
\t\t\t\t{ntw_debug_config_id} /* Debug */,
\t\t\t\t{ntw_release_config_id} /* Release */,
\t\t\t);
\t\t\tdefaultConfigurationIsVisible = 0;
\t\t\tdefaultConfigurationName = Release;
\t\t}};'''

    content = content.replace(
        '/* End XCConfigurationList section */',
        ntw_config_list + '\n/* End XCConfigurationList section */'
    )

    # -----------------------------------------------------------------------
    # Write the result
    # -----------------------------------------------------------------------

    write_project(content)
    print(f"\nSaved {PROJECT_PATH}")
    print(f"\nCreated NottawaApp target with ID {ntw_target_id}")
    print("Both dude_wake_up and NottawaApp targets are now configured.")
    print("\nNOTE: Open in Xcode and verify both targets appear in the scheme selector.")


if __name__ == '__main__':
    main()
