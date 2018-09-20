# How to Create a Release of OpenCensus C++ (for Maintainers Only)

## Tagging the Release

The first step in the release process is to create a release branch, bump
versions, and create a tag for the release. Our release branches follow the
naming convention of `v<major>.<minor>.x`, while the tags include the patch
version `v<major>.<minor>.<patch>`. For example, the same branch `v0.1.x` would
be used to create all `v0.1` tags (e.g. `v0.1.0`, `v0.1.1`).

In this section upstream repository refers to the main opencensus-cpp github
repository.

Before any push to the upstream repository you need to create a [personal access
token](https://help.github.com/articles/creating-a-personal-access-token-for-the-command-line/).

1.  Create the release branch and push it to GitHub:

    ```bash
    $ MAJOR=0 MINOR=1 PATCH=0 # Set appropriately for new release
    $ VERSION_FILES=(
        opencensus/common/version.h
      )
    $ git checkout -b v$MAJOR.$MINOR.x master
    $ git push upstream v$MAJOR.$MINOR.x
    ```

    The branch will be automatically protected by the GitHub branch protection rule for release
    branches.

2.  For `master` branch:

    -   Change version files to the next minor snapshot (e.g. `0.2.0-dev`).

    ```bash
    $ git checkout -b bump-version master
    # Change version to next minor (and keep -dev)
    $ sed -i 's/\(.*OPENCENSUS_VERSION.*\)[0-9]\+\.[0-9]\+\.[0-9]\+/\1'$MAJOR.$((MINOR+1)).0'/' \
      "${VERSION_FILES[@]}"
    $ tools/presubmit.sh
    $ git commit -a -m "Start $MAJOR.$((MINOR+1)).0 development cycle"
    ```

    -   Go through PR review and push the master branch to GitHub:

    ```bash
    $ git checkout master
    $ git merge --ff-only bump-version
    $ git push upstream master
    ```

3.  For `vMajor.Minor.x` branch:

    -   Change version files to remove "-dev" for the next release
        version (e.g. `0.4.0`). Commit the result and make a tag:

    ```bash
    $ git checkout -b release v$MAJOR.$MINOR.x
    # Change version to remove -dev
    $ sed -i 's/\(.*OPENCENSUS_VERSION.*[0-9]\+\.[0-9]\+\.[0-9]\+\)-dev/\1/' \
      "${VERSION_FILES[@]}"
    $ tools/presubmit.sh
    $ git commit -a -m "Bump version to $MAJOR.$MINOR.$PATCH"
    $ git tag -a v$MAJOR.$MINOR.$PATCH -m "Version $MAJOR.$MINOR.$PATCH"
    ```

    -   Change root build files to the next snapshot version (e.g.
        `0.4.1-dev`). Commit the result:

    ```bash
    # Change version to next patch and add -dev
    $ sed -i 's/\(.*OPENCENSUS_VERSION.*\)[0-9]\+\.[0-9]\+\.[0-9]\+/\1'$MAJOR.$((MINOR+1)).$((PATCH+1))-dev'/' \
      "${VERSION_FILES[@]}"
    $ tools/presubmit.sh
    $ git commit -a -m "Bump version to $MAJOR.$MINOR.$((PATCH+1))-dev"
    ```

    -   Go through PR review and push the release tag and updated release branch
        to GitHub:

    ```bash
    $ git checkout v$MAJOR.$MINOR.x
    $ git merge --ff-only release
    $ git push upstream v$MAJOR.$MINOR.$PATCH
    $ git push upstream v$MAJOR.$MINOR.x
    ```

4. Write release notes
   -   Go to the [release page][RELEASE_LINK], draft a new release and publish
       it after review.


[RELEASE_LINK]: https://github.com/census-instrumentation/opencensus-cpp/releases
