#!/bin/bash
PLUGIN_PATH="$1"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [ -f "$SCRIPT_DIR/sign_plugin.sh" ]; then
    echo "Signing plugin"
    sh "$SCRIPT_DIR/sign_plugin.sh" "$PLUGIN_PATH"
else
  echo "If you want to sign, create a sign_plugin.sh script and call this :"
  echo "codesign -f -s \"Your Identity\" \"$PLUGIN_PATH\""
fi