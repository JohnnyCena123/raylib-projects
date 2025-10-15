if [ "$#" -eq "0" ]; then
	git_branch="$(git rev-parse --abbrev-ref HEAD)"
else
	for arg in "$@"; do
		case "$arg" in
			--git-branch=*) git_branch="${arg#--git-branch=}" ;;
			*)
				echo "Unknown argument: $arg" >&2
				exit 2
				;;
		esac
	done
fi
cd "$(dirname "$0")/../../bin/" || exit 1
cd "$git_branch/" && echo "local build detected" > /dev/null || echo "likely github actions" > /dev/null
directory="$git_branch-web-app-zip/"
mkdir -p "$directory"
cp "$git_branch".{js,wasm,html,data} "$directory"
cd "$directory" || exit 1
mv "$git_branch.html" index.html
zip -r "../$git_branch.zip" "$git_branch".{js,wasm,data} ./index.html
