#!/bin/bash

# Nottawa Release Environment Setup Script
# This script helps set up the environment for release builds

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }

echo "========================================="
echo "Nottawa Release Environment Setup"
echo "========================================="
echo ""

# Check for Xcode
print_info "Checking Xcode installation..."
if xcode-select -p &> /dev/null; then
    XCODE_VERSION=$(xcodebuild -version | head -n1)
    print_success "Found $XCODE_VERSION"
else
    print_error "Xcode is not installed or command line tools are not selected"
    echo "Run: xcode-select --install"
    exit 1
fi

# Check for Developer ID Certificate
print_info "Checking for Developer ID certificates..."
DEVELOPER_CERTS=$(security find-identity -v -p codesigning | grep "Developer ID Application")
if [ -z "$DEVELOPER_CERTS" ]; then
    print_warning "No Developer ID Application certificate found"
    echo "You'll need to:"
    echo "1. Enroll in the Apple Developer Program"
    echo "2. Create a Developer ID Application certificate"
    echo "3. Install it in your keychain"
else
    print_success "Found Developer ID certificates:"
    echo "$DEVELOPER_CERTS"
fi

# Check for Sparkle framework
print_info "Checking for Sparkle framework..."
if [ -d "./lib/Sparkle.framework" ]; then
    print_success "Sparkle framework found"

    # Check for Sparkle signing tool
    if [ -f "./lib/Sparkle.framework/bin/sign_update" ]; then
        print_success "Sparkle signing tool found"
    else
        print_warning "Sparkle signing tool not found at ./lib/Sparkle.framework/bin/sign_update"
    fi

    # Check for Sparkle private key in keychain
    print_info "Checking for Sparkle private key in keychain..."
    if security find-generic-password -a "ed25519" &> /dev/null; then
        print_success "Sparkle private key found in keychain"
    else
        print_warning "Sparkle private key not found in keychain"
        echo "To add your Sparkle private key to the keychain:"
        echo "1. Generate keys: ./lib/Sparkle.framework/bin/generate_keys"
        echo "2. Add to keychain: security add-generic-password -a ed25519 -s 'Sparkle Private Key' -w 'YOUR_PRIVATE_KEY'"
    fi
else
    print_warning "Sparkle framework not found"
    echo "Download from: https://github.com/sparkle-project/Sparkle/releases"
fi

# Check for notarization credentials
print_info "Checking for notarization setup..."
if [ -z "$APPLE_ID" ]; then
    print_warning "APPLE_ID environment variable not set"
    echo "To set up notarization:"
    echo "1. Export your Apple ID: export APPLE_ID='your-apple-id@example.com'"
    echo "2. Store your app-specific password in keychain:"
    echo "   xcrun notarytool store-credentials 'AC_PASSWORD' --apple-id 'your-apple-id@example.com' --team-id 'YOUR_TEAM_ID'"
else
    print_success "APPLE_ID is set: $APPLE_ID"

    # Check for app-specific password in keychain
    if security find-generic-password -s "AC_PASSWORD" &> /dev/null; then
        print_success "App-specific password found in keychain"
    else
        print_warning "App-specific password not found in keychain"
        echo "Store it with:"
        echo "xcrun notarytool store-credentials 'AC_PASSWORD' --apple-id '$APPLE_ID' --team-id 'YOUR_TEAM_ID'"
    fi
fi

# Create .env.release template if it doesn't exist
if [ ! -f ".env.release" ]; then
    print_info "Creating .env.release template..."
    cat > .env.release << 'EOF'
# Nottawa Release Environment Variables
# Copy this file and fill in your values
# Source it before running release.sh: source .env.release

# Apple Developer Account
export APPLE_ID="your-apple-id@example.com"
export TEAM_ID="YOUR_TEAM_ID"

# Optional: Explicitly set Developer ID (auto-detected if not set)
# export DEVELOPER_ID="Developer ID Application: Your Name (TEAM_ID)"

# Server paths for deployment
export NOTTAWA_SERVER_DIR="$HOME/dev/nottawa-server"
export NOTTAWA_WEBSITE_URL="https://nottawa.app"
EOF
    print_success "Created .env.release template - please edit with your values"
else
    print_info ".env.release already exists"
fi

# Summary
echo ""
echo "========================================="
echo "Setup Summary"
echo "========================================="

# Count issues
ISSUES=0
[ -z "$DEVELOPER_CERTS" ] && ((ISSUES++))
[ ! -d "./lib/Sparkle.framework" ] && ((ISSUES++))
[ -z "$APPLE_ID" ] && ((ISSUES++))

if [ $ISSUES -eq 0 ]; then
    print_success "Your environment is ready for release builds!"
    echo ""
    echo "To build a release:"
    echo "1. source .env.release  # If you have custom settings"
    echo "2. ./release.sh VERSION"
    echo ""
    echo "Example: ./release.sh 1.0.3"
else
    print_warning "There are $ISSUES issue(s) to resolve before you can build releases"
    echo ""
    echo "Required for full release process:"
    echo "• Developer ID Application certificate"
    echo "• Apple Developer Program membership"
    echo "• Sparkle framework for auto-updates"
    echo "• Notarization credentials"
    echo ""
    echo "You can still build unsigned DMGs with:"
    echo "./release.sh VERSION --skip-notarization --skip-sparkle"
fi

echo ""
echo "For more information, see:"
echo "• Apple Developer: https://developer.apple.com/"
echo "• Sparkle: https://sparkle-project.org/"
echo "• Notarization: https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution"