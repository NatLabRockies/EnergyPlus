Developer Scripts
=================

This folder contains scripts that are used for development.  I imagine these are mostly automatically run by CMake, but not necessarily.

## Pre-commit

These scripts are registered as [pre-commit](https://pre-commit.com/) hooks.

As long as you have a python (3.12 currently to match the python we ship with) environment ready you can do

```shell
pip install pre-commit
```

To register the pre-commit hooks as a git pre-commit hook, so it runs on your **staged** files every time you try to commit do:

```shell
pre-commit install
```

### Advanced usage

(if you ever need to bypass this, do `git commit --no-verify` (or `-n` for short))

If you want to run for all files and not just the staged files, you can do

```shell
pre-commit run --all-files
```

You can also run a single hook

```shell
pre-commit run --verbose license-check --all-files
```


Note: `check_for_enum_scope_usage.py` is slow so it's added as a "manual" hook stage, so it doesn't run on each pre-commit.

To force run it,

```
pre-commit run --verbose --hook-stage manual --all-files
```
