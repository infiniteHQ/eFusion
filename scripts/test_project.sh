mkdir -p ../lib/vortex/tests/project/.vx/modules
cp -r ../dist/* ../lib/vortex/tests/project/.vx/modules

VERSION=$(cat ../lib/vortex/version.conf)

SCRIPT_DIR=$(dirname "$(realpath "$0")")

VORTEX_PATH="${SCRIPT_DIR}/../lib/vortex/build/dist/${VERSION}/bin/"
PROJECT_PATH="${SCRIPT_DIR}/../lib/vortex/tests/project"

SESSION_ID="editor-$(date +%m-%d-%Y-%H-%M-%S)-$(shuf -i 1000-9999 -n 1)"

cd "$PROJECT_PATH" && bash "$VORTEX_PATH/handle_crash.sh" \
  "$HOME/.vx/sessions/${SESSION_ID}/crash/core_dumped.txt" \
  "$VORTEX_PATH/vortex" --editor --session_id="\"$SESSION_ID\"" \
  ::END:: \
  "$VORTEX_PATH/vortex" --crash --session_id="\"$SESSION_ID\""
