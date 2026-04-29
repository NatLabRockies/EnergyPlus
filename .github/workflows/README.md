# Overview

This folder is, of course, where our GitHub Action workflows live.
I made a significant effort to pull us completely over from Decent CI to GitHub Actions, in PR #10683.
While everything looked good in testing, it seems the number of CI runs was too much for the NREL org.
I reverted it right away, and am going to retry later, probably with the help of some self-hosted runners.
This README is really just a place to drop current status while I figure out what to add back in next.

# Current Configuration

- build_checksums.yml
  - Runs on demand or on branches named "checksum"
- build_documentation.yml
  - Runs on Windows and Linux to test MikTeX and TeXLive
  - Linux takes 5 minutes and Windows takes 9 minutes
- build_wheels.yml
  - Runs on demand, on Linux only for now
- release_documentation.yml
  - Only works on tags
  - Build and publish the PDF documentation.
- release_linux.yml, release_mac.yml, release_windows.yml
  - Only work on tags
  - Build and publish the platform-specific release packages for Linux, macOS, and Windows
- test_code_integrity.yml
  - Runs on Linux, about 22 minutes
- test_debug_builds.yml
  - Currently not running at all because of runtime
  - Maybe push these onto self-hosted runners soon
- test_develop_commits.yml
  - Only running Mac builds, as they are fast
  - Then maybe push these onto self-hosted runners for Windows and Ubuntu
- test_epjson.yml
  - Currently running just on Windows to save CI time
  - In the future, probably expand it back out to all platforms
  - Approximately 3 minute runtime
- test_pull_requests.yml
  - In this PR I am adding Mac builds back in with new regressions
  - In the future expand these onto self-hosted runners for Windows and Ubuntu
- verify_pr_labels.yml
  - Runs on Linux, and less than a minute

Thus for a typical PR push, it will start up:

- Documentation - Windows: 9 minutes
- Documentation - Linux: 5 minutes
- Code integrity - Linux: 22 minutes
- EpJSON - Windows: 3 minutes
- PR Labeling - Linux: 1 minute
- Test PR Including Regressions - Mac: 49 minutes

Which is a total of Windows: 12 minutes, Linux: 38 minutes, Mac: 49 minutes.

Decent CI will be supplementing with:

- Linux Debug Unit Test Coverage
- Linux Debug Integration Test Coverage
- Linux Release Build and Test Without Regressions
- Windows Release Build and Test Without Regressions

# TODO

- Could add some logic in the documentation workflow to only run if docs changed
  - on: push: paths: '**.tex'
  - But...we would also want to consider other changes that affect docs, like CMake config, versioning, etc.
- Get all docs to ReadTheDocs and eliminate our TeX entirely.................................
- Set up Self-Hosted runners and see how that goes
