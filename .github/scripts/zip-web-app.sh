git_branch="$(git rev-parse --abbrev-ref HEAD)"
cd "$(dirname "$0")/../../bin/" || exit 1
cd "$git_branch/" && echo "local build detected" > /dev/null || echo "likely github actions" > /dev/null
directory="$git_branch-web-app-zip/"
mkdir -p "$directory"
cp "$git_branch".{js,wasm,html,data} "$directory"
cd "$directory" || exit 1
mv "$git_branch.html" index.html
zip -r "../$git_branch.zip" "$git_branch".{js,wasm,data} ./index.html
