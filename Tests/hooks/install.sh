#!/bin/sh
# Installs the pre-push hook (the macOS self-test before every push) into
# this clone's hooks (shared by its worktrees). Run once per clone.
repo=$(cd "$(dirname "$0")/../.." && pwd)
hooks=$(cd "$repo" && cd "$(git rev-parse --git-common-dir)" && pwd)/hooks
mkdir -p "$hooks" && cp "$repo/Tests/hooks/pre-push" "$hooks/pre-push" && chmod +x "$hooks/pre-push" &&
  echo "Installed $hooks/pre-push"
