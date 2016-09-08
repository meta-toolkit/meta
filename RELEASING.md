# Releasing MeTA Versions

This document contains a checklist for releasing a version of MeTA so we
follow a consistent releasing process.

1. Pick a version number. MeTA releases (Major.Minor.Patch) ought to be API
   compatible with other releases that share the same Major and Minor
   version numbers but different Patch versions.

   Major API changes (like new libraries or toolkit-wide backwards
   incompatible API changes) increment the Major release number. Minor API
   changes (like enhancements) increment the Minor release number. Patch
   versions should be released only for bug fixes.

2. Ensure `CHANGELOG.md` is up to date.

   If there are *any* breaking changes, mention these explicitly. If there
   are migration strategies to work around these breaking changes, provide
   a brief explanation (or a link to explain them).

3. If there are major *or* minor API changes, ensure that the documentation
   on the website (meta-toolkit/meta-toolkit.org) is correct.

   Update Doxygen as necessary.

4. Ensure that the build is passing on both Travis (Linux + OS X) and
   Appveyor (Windows/MinGW-w64).

5. Merge branch `develop` into `master` with a commit message

   > Merge branch 'develop' for MeTA vX.Y.Z

   Use `git merge develop --no-ff` to create a merge commit.

6. Tag the merge commit. The tag should be both annotated *and* signed:

   ```
   git tag -as vX.Y.Z
   ```

   The tag message should be the contents of the CHANGELOG for that
   version. Remove unnecessary markdown syntax like header markers and code
   blocks. Backticks can stay.

7. Push the merge and the tags to GitHub:

   ```
   git push --follow-tags
   ```

8. Create a release on GitHub using the new tag. Its title should be "MeTA
   vX.Y.Z".

   The contents of the message should be exactly the same as the CHANGELOG
   entry for that release.

9. Upload the model files and include a section in the GitHub release notes
   containing their sha256 sums.
