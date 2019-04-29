# How to Create a Release of OpenCensus C++ (for Maintainers Only)

In this example, pretend branch `master` currently has
[`OPENCENSUS_VERSION`](opencensus/common/version.h) set to "0.1.0-dev" -- which
means we are creating release `v0.1.0`.

1. Create a new branch `v0.1.x`.
1. Bump branch `master` to the next minor: `"0.2.0-dev"`.
   ([example](https://github.com/census-instrumentation/opencensus-cpp/pull/271))
1. On branch `v0.1.x`: pin build dependencies.
   ([example](https://github.com/census-instrumentation/opencensus-cpp/pull/273))
1. On branch `v0.1.x`: bump version to `"0.1.0"` (i.e. not `-dev` anymore)
   ([example](https://github.com/census-instrumentation/opencensus-cpp/pull/274))
1. Create a release with tag `v0.1.0`.
   ([example](https://github.com/census-instrumentation/opencensus-cpp/releases/tag/v0.3.0))
1. On branch `v0.1.x`: bump version to `"0.1.1-dev"` (i.e. the next `-dev`)
   ([example](https://github.com/census-instrumentation/opencensus-cpp/pull/275))

## Detailed instructions

Our release branches follow the
naming convention of `v<major>.<minor>.x`, while the tags include the patch
version `v<major>.<minor>.<patch>`. For example, the same branch `v0.1.x` would
be used to create all `v0.1` tags (e.g. `v0.1.0`, `v0.1.1`).

In this section, the remote called `upstream` refers to the official opencensus-cpp repository:
`git@github.com:census-instrumentation/opencensus-cpp.git`

The remote called `origin` refers to your fork, which you use to send PRs, e.g.:
`git@github.com:$USER/opencensus-cpp.git`

If you are using `https` instead of `ssh` as above,
before any push to the upstream repository you need to create a [personal access
token](https://help.github.com/articles/creating-a-personal-access-token-for-the-command-line/).

1.  Create the release branch and push it to GitHub:

    ```bash
    $ MAJOR=0 MINOR=1 PATCH=0 # Set appropriately for new release
    $ VERSION_FILES=(
        opencensus/common/version.h
      )
    $ git checkout -b v$MAJOR.$MINOR.x remotes/upstream/master
    $ git push upstream v$MAJOR.$MINOR.x
    ```

    The branch will be automatically protected by the GitHub branch protection
    rule for release branches.

1.  Bump branch `master` to the next minor version: `0.2.0-dev`

    ```bash
    $ git checkout -b bump-version remotes/upstream/master
    # Change version to next minor (and keep -dev)
    $ sed -i 's/\(.*OPENCENSUS_VERSION.*\)[0-9]\+\.[0-9]\+\.[0-9]\+/\1'$MAJOR.$((MINOR+1)).0'/' \
      "${VERSION_FILES[@]}"
    $ tools/presubmit.sh
    $ git commit -a -m "Start $MAJOR.$((MINOR+1)).0 development cycle."
    ```
    Also bump the version in `CMakeLists.txt`. Leave out the `-dev` suffix
    because CMake doesn't support it.

    Push, then make a PR against the **`master`** branch:

    ```bash
    $ git push origin bump-version
    ```

1. Switch to the release branch and pin BUILD dependencies.

    ```bash
    $ git checkout -b deps remotes/upstream/v$MAJOR.$MINOR.x
    ```

    One day, this will be more automated. In the meantime,
    run `tools/pin_deps.py` and edit the `WORKSPACE` file accordingly.

    Likewise update the `cmake/OpenCensusDeps.cmake` file.

    Run `tools/presubmit.sh` to test building with bazel, and follow the
    [CMake README](cmake/README.md) for CMake.

    Push, then make a PR against the **`release`** branch, not the `master`
    branch: (important!)

    ```bash
    $ git push origin deps
    ```

1. Bump the release branch to the release version. (remove `-dev`)

    ```bash
    $ git checkout -b release remotes/upstream/v$MAJOR.$MINOR.x
    # Change version to remove -dev
    $ sed -i 's/\(.*OPENCENSUS_VERSION.*[0-9]\+\.[0-9]\+\.[0-9]\+\)-dev/\1/' \
      "${VERSION_FILES[@]}"
    $ tools/presubmit.sh
    $ git commit -a -m "Bump version to $MAJOR.$MINOR.$PATCH."
    ```

    Push, then make a PR against the **`release`** branch, not the `master`
    branch: (important!)

    ```bash
    $ git push origin release
    ```

1. Create the release with tag `v0.1.0` using the branch `v0.1.x`

    * Go to the [releases][RELEASE_LINK] page on GitHub.
    * Click "Draft a new release."
    * Set the "Tag version" to `v0.1.0`.
    * Set the "Target" to `v0.1.x`. (important!)
    * Set the "Release title" to "v0.1.0 Release."
    * Fill out the description with highlights.
    * Click "Publish release."

1. Bump the release branch to the next `-dev` version.

    ```bash
    $ git checkout -b release remotes/upstream/v$MAJOR.$MINOR.x
    # Change version to next patch and add -dev
    $ sed -i 's/\(.*OPENCENSUS_VERSION.*\)[0-9]\+\.[0-9]\+\.[0-9]\+/\1'$MAJOR.$MINOR.$((PATCH+1))-dev'/' \
      "${VERSION_FILES[@]}"
    $ tools/presubmit.sh
    $ git commit -a -m "Bump version to $MAJOR.$MINOR.$((PATCH+1))-dev."
    ```

    Also bump the version in `CMakeLists.txt`. Leave out the `-dev` suffix
    because CMake doesn't support it.

    Push, then make a PR against the **`release`** branch, not the `master`
    branch: (important!)

    ```bash
    $ git push origin release
    ```

1. You're done! `\o/`

[RELEASE_LINK]: https://github.com/census-instrumentation/opencensus-cpp/releases
