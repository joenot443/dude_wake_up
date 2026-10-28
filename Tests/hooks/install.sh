#!/bin/sh
# Installs the pre-push hook (the macOS self-test before every push) into
# this clone's .git/hooks. Run once per clone.
repo=$(cd "$(dirname "$0")/../.." && pwd)
cp "$repo/Tests/hooks/pre-push" "$repo/.git/hooks/pre-push" && chmod +x "$repo/.git/hooks/pre-push" &&
  echo "Installed $repo/.git/hooks/pre-push"
