#!/bin/bash

if ! command -v brew &> /dev/null; then
    echo "Homebrew is not installed. Please install it first: https://brew.sh/"
    exit 1
fi

echo "Installing dependencies for OS development on Mac..."
brew install nasm
brew install i686-elf-binutils
brew install i686-elf-gcc
brew install qemu

echo "Setup complete."
