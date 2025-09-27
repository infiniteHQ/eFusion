#!/bin/sh

rm -rf ../dist
rm -rf ../build

rm -rf ../lib/vortex/tests/project/.vx/modules/
mkdir -p ../build
cd ../build

cmake ..
make

MODULE_JSON_PATH="../module.json"
NAME=$(jq -r .name $MODULE_JSON_PATH)
VERSION=$(jq -r .version $MODULE_JSON_PATH)
FOLDER_NAME="$NAME-$VERSION"

mkdir -p "../dist/$FOLDER_NAME"

cp -r ../build ../dist/$FOLDER_NAME/
cp -r ../lib ../dist/$FOLDER_NAME/ 2>/dev/null || true
cp -r ../assets ../dist/$FOLDER_NAME/ 2>/dev/null || true
cp ../module.json ../dist/$FOLDER_NAME/

rm -rf "../dist/$FOLDER_NAME/build/CMakeFiles"
rm -rf "../dist/$FOLDER_NAME/scripts"
rm -rf "../dist/$FOLDER_NAME/.git"
rm -rf "../dist/$FOLDER_NAME/.vscode"
rm -rf "../dist/$FOLDER_NAME/lib"
rm "../dist/$FOLDER_NAME/build/cmake_install.cmake" 2>/dev/null || true
rm "../dist/$FOLDER_NAME/build/CMakeCache.txt" 2>/dev/null || true
rm "../dist/$FOLDER_NAME/build/dist.tar.gz" 2>/dev/null || true
rm "../dist/$FOLDER_NAME/build/Makefile" 2>/dev/null || true
rm "../dist/$FOLDER_NAME/.gitmodules" 2>/dev/null || true
rm "../dist/$FOLDER_NAME/CMakeLists.txt" 2>/dev/null || true

TAR_NAME="../build/$FOLDER_NAME.tar.gz"
tar -czf "$TAR_NAME" -C ../dist "$FOLDER_NAME"

echo "Archive créée : $TAR_NAME"