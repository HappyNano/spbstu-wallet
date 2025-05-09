#!/bin/bash

PROJECT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"

if [[ -z $1 ]]; 
then
  ROOT_DIR=$(pwd)
else
  ROOT_DIR="$(pwd)/$1"
fi
# All c/cpp files
FILES=$(find "$ROOT_DIR" -name '*.cpp' -o -name '*.h' -o -name '*.c' -o -name '*.hpp')

# Colors
ORANGE='\033[0;33m'
GREEN='\033[0;32m'
NC='\033[0m'

for FILE in $FILES; do
  TEMP_FILE=$(mktemp)
  clang-format --assume-filename=$PROJECT_ROOT.clang-format "$FILE" > "$TEMP_FILE"

  if cmp -s "$FILE" "$TEMP_FILE"; then
    echo -e "${ORANGE}Not changed: $FILE${NC}"
  else
    echo -e "${GREEN}Changed: $FILE${NC}"
    mv "$TEMP_FILE" "$FILE"
  fi

  [ -f "$TEMP_FILE" ] && rm "$TEMP_FILE"
done
