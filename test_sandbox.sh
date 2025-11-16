#!/bin/bash
# Test sandbox functionality

echo "Testing sandbox with mask for open..."
echo "sandbox 32768 - cat README" | timeout 10 make qemu-nox 2>&1 | grep -A5 "sandbox 32768"
