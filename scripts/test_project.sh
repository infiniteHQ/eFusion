#!/bin/sh

SCRIPT_DIR=$(dirname "$(realpath "$0")")
MODULE_ROOT="${SCRIPT_DIR}/.."
VORTEX_ROOT="${MODULE_ROOT}/lib/vortex"

VXPATH_FILE="${MODULE_ROOT}/vxpath"

if [ -f "$VXPATH_FILE" ]; then
  VXPATH=$(head -n 1 "$VXPATH_FILE" | tr -d '\r' | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' -e 's/^"\(.*\)"$/\1/')

  if [ -n "$VXPATH" ]; then
    case "$VXPATH" in
      "~"*) VXPATH="${HOME}${VXPATH#\~}" ;;
    esac

    case "$VXPATH" in
      /*) ;;
      *) VXPATH="${MODULE_ROOT}/${VXPATH}" ;;
    esac

    if [ -d "$VXPATH" ]; then
      VXPATH=$(cd "$VXPATH" && pwd)
      EXT_VERSION=$(cat "$VXPATH/version.conf" 2>/dev/null)

      if [ -n "$EXT_VERSION" ] && [ -d "$VXPATH/build/dist/${EXT_VERSION}/bin" ]; then
        VORTEX_ROOT="$VXPATH"
        echo "Using external Vortex from vxpath: $VORTEX_ROOT"
      else
        echo "Warning: vxpath '$VXPATH' has no valid Vortex build, falling back to lib/vortex" >&2
      fi
    else
      echo "Warning: vxpath '$VXPATH' does not exist, falling back to lib/vortex" >&2
    fi
  fi
fi

VERSION=$(cat "${VORTEX_ROOT}/version.conf")

VORTEX_PATH="${VORTEX_ROOT}/build/dist/${VERSION}/bin/"
PROJECT_PATH="${VORTEX_ROOT}/tests/project"

mkdir -p "${PROJECT_PATH}/.vx/modules"
cp -r "${MODULE_ROOT}"/dist/* "${PROJECT_PATH}/.vx/modules"

SESSION_ID="editor-$(date +%m-%d-%Y-%H-%M-%S)-$(shuf -i 1000-9999 -n 1)"

cd "$PROJECT_PATH" && bash "$VORTEX_PATH/handle_crash.sh" \
  "$HOME/.vx/sessions/${SESSION_ID}/crash/core_dumped.txt" \
  "$VORTEX_PATH/vortex" --editor --session_id="\"$SESSION_ID\"" \
  ::END:: \
  "$VORTEX_PATH/vortex" --crash --session_id="\"$SESSION_ID\""