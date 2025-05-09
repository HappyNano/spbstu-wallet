#!/bin/bash

PROJECT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"

if [ $# -eq 0 ]; then
  adb install -r $PROJECT_ROOT/platforms/android/app/build/outputs/apk/debug/app-debug.apk
else
  adb install -s $1 -r $PROJECT_ROOT/platforms/android/app/build/outputs/apk/debug/app-debug.apk
fi

