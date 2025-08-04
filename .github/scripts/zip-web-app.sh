git_branch="$(git rev-parse --abbrev-ref HEAD)"
cd "$(dirname "$0")/../../bin/$git_branch/" || cd "$(dirname "$0")/../../bin/" || exit 1
directory="$git_branch-web-app-zip/"
mkdir -p "$directory"
cp "$git_branch".{js,wasm,html,data} "$directory"
cd "$directory" || exit 1
mv "$git_branch.html" index.html
zip -r "../$git_branch.zip" "$git_branch".{js,wasm,data} ./index.html
