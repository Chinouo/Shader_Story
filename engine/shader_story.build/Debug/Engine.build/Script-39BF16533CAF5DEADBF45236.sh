#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/dtup/Developer/vscode-workspace/shader_story/engine
  /opt/homebrew/Cellar/cmake/3.23.1/bin/cmake -E cmake_symlink_library /Users/dtup/Developer/vscode-workspace/shader_story/engine/Debug/libEngine.dylib /Users/dtup/Developer/vscode-workspace/shader_story/engine/Debug/libEngine.dylib /Users/dtup/Developer/vscode-workspace/shader_story/engine/Debug/libEngine.dylib
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/dtup/Developer/vscode-workspace/shader_story/engine
  /opt/homebrew/Cellar/cmake/3.23.1/bin/cmake -E cmake_symlink_library /Users/dtup/Developer/vscode-workspace/shader_story/engine/Release/libEngine.dylib /Users/dtup/Developer/vscode-workspace/shader_story/engine/Release/libEngine.dylib /Users/dtup/Developer/vscode-workspace/shader_story/engine/Release/libEngine.dylib
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/dtup/Developer/vscode-workspace/shader_story/engine
  /opt/homebrew/Cellar/cmake/3.23.1/bin/cmake -E cmake_symlink_library /Users/dtup/Developer/vscode-workspace/shader_story/engine/MinSizeRel/libEngine.dylib /Users/dtup/Developer/vscode-workspace/shader_story/engine/MinSizeRel/libEngine.dylib /Users/dtup/Developer/vscode-workspace/shader_story/engine/MinSizeRel/libEngine.dylib
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/dtup/Developer/vscode-workspace/shader_story/engine
  /opt/homebrew/Cellar/cmake/3.23.1/bin/cmake -E cmake_symlink_library /Users/dtup/Developer/vscode-workspace/shader_story/engine/RelWithDebInfo/libEngine.dylib /Users/dtup/Developer/vscode-workspace/shader_story/engine/RelWithDebInfo/libEngine.dylib /Users/dtup/Developer/vscode-workspace/shader_story/engine/RelWithDebInfo/libEngine.dylib
fi

