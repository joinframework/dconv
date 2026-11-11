#!/usr/bin/env bash
missing () {
    echo "format hook: $1 is missing, nothing was formatted. Ask the user to install it (AGENTS.md, Definition of done)." >&2
    exit 2
}
command -v jq >/dev/null || missing jq
file=$(jq -r '.tool_input.file_path // empty')
case "$file" in
    *.hpp|*.cpp)
        command -v clang-format >/dev/null || missing clang-format
        [ -f "$file" ] || exit 0
        clang-format -i "$file" || { echo "format hook: clang-format failed on $file" >&2; exit 2; }
        ;;
esac
exit 0
