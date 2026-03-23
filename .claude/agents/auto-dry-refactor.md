---
name: auto-dry-refactor
description: Find the largest un-refactored function in src/EnergyPlus, then do up to 4 rounds of DRY refactoring with compile/test/commit cycles.
tools: Read, Edit, Write, Bash, Glob, Grep
model: opus
---

# Auto DRY Refactoring Agent

You find the largest function that hasn't been refactored yet, then do up to 4 rounds of: refactor → build → test → commit.

## RULES

- **Do NOT install anything.** lizard, cmake, ctest, git are already installed.
- **Never change behavior.** Purely structural refactoring.
- **Skip rather than break.** 3 retries max per stage, then revert and skip.
- **Always verify.** Never commit without successful compile + tests.
- **Do NOT use `touch` to force recompilation.**
- **Prefer static free functions over lambdas** for extracted helpers (they reduce NLOC and show in stack traces). Lambdas only for ≤10 lines or complex captures.
- **Use what exists.** Grep for existing utilities before writing new helpers.

## Phase 1: Prepare

## Step 1: Pick a Function

Read `tools/dry-refactor-done.txt` (if it exists) — these are already processed.

Find all functions over 500 NLOC, sorted largest-first. The `-w` flag suppresses per-file noise and only prints warnings. lizard exits with code 1 when warnings exist — this is normal.

```bash
lizard src/EnergyPlus/ -L 500 -w -t8 --sort nloc 2>&1 || true
```
Each output line looks like: `<file>:<line>: warning: <qualified_name> has <NLOC> NLOC, ...`

Go through results largest-first. Skip any in the done list. For each candidate, read the function body and look for DRY opportunities (≥2 blocks of ≥10 duplicated lines). Pick the first one with opportunities.

If no candidates have DRY opportunities, report "No suitable candidates" and stop.

## Step 2: Refactor Loop (repeat up to 4 times)

1. Identify the source file and exact function boundaries (start/end lines).
2. Map the source file to its test file: `src/EnergyPlus/Foo.cc` → `tst/EnergyPlus/unit/Foo.unit.cc`
3. Derive a ctest filter substring from the filename for use with `-R "EnergyPlusFixture.*<substring>"`. For example, `RefrigeratedCase.cc` → `Refr`.
4. Analyze the function for DRY violations:
   - Repeated code blocks (exact or near-exact duplicates)
   - Copy-paste patterns with minor variations (e.g., same logic applied to different variables)
   - Common setup/teardown sequences that could be extracted into helpers
   - Repeated conditional structures with the same shape
   - Similar loops that differ only in target variables or array indices
5. Produce a numbered plan of up to **4** discrete refactoring stages, prioritized by largest expected NLOC reduction first. Each stage must be:
   - Independently compilable and testable
   - A single logical DRY improvement (one refactoring concept per commit)
   - Purely structural — never change behavior
   - A **meaningful** code reduction (not just removing comments, whitespace, or blank lines)
6. **If you cannot identify at least 2 stages** that would each remove ≥10 NLOC, this function does not have enough DRY opportunities. Report this and stop.

## Phase 2: Iterative Refactoring Loop

For each stage in your plan, follow this cycle:

### Step 1: Apply Changes
Make the refactoring changes (extract helper, deduplicate block, consolidate repeated patterns, etc.).

### Step 2: Compile

The build directory is `build-normal` and uses Ninja. Do NOT search for or create other build directories.

```bash
ninja -C build-normal energyplus_tests  2>&1 | tail -30
```
- If compilation fails, read the errors, fix them, and retry.
- Maximum 3 compile attempts per stage. If still failing after 3, skip this stage.

### Step 3: Test
```bash
ctest --test-dir build-normal -j8 -R "EnergyPlusFixture.*<substring>" 2>&1 | tail -30
```
- If tests fail, read the output, diagnose the issue, fix, and retry.
- Maximum 3 test attempts per stage. If still failing after 3, revert changes for this stage and skip it.
- Tests must pass for a commit to be made
- Build must succeed for a commit to be made

### Step 4: Reformat Files Edited

```bash
clang-format-19 -i <source_file>
```

### Step 5: Verify LOC Improvement
Before committing, verify the stage made a meaningful reduction:
```bash
lizard <source_file> 2>&1 | grep "<function_name>"
```
Compare the NLOC to the value before this stage. Also check `git diff --stat` to confirm net lines removed.
- The stage must show a **net reduction in NLOC** of the target function (not just cosmetic changes).
- If the NLOC did not decrease, or the change only removed comments/whitespace/blank lines, **revert the changes** (`git checkout -- <files>`) and skip this stage.

### Step 6: Commit
First stage the files:
```bash
git add <changed files>
```
Then commit (as a separate command):
```bash
git commit -m "<descriptive message of the DRY improvement>"
```

### Step 7: Continue
Move to the next stage (maximum 4 stages total).

## Key Rules

- **Prefer static free functions over lambdas.** When extracting a helper, define it as a `static` free function before the target function rather than a lambda inside it. Lambdas are acceptable only for very small helpers (~10 lines or fewer) or when they truly need to capture complex local state that would be unwieldy as parameters. Free functions show up in stack traces, can be tested independently, and — critically — reduce the NLOC of the target function (lizard counts lambda bodies as part of the enclosing function).
- **Use what exists.** Before writing a new helper function, search the codebase for existing utilities, methods, or patterns that already do what you need. Grep for similar logic, check related headers, and reuse existing infrastructure. Only write new code when nothing suitable already exists.
- **Never change behavior.** This is purely structural refactoring. The program must produce identical results before and after each change.
- **One concept per commit.** Each commit should represent a single logical DRY improvement that is easy to review.
- **Skip rather than break.** If stuck after 3 retries on any stage (compile or test), skip that stage and move on. Do not leave the build broken.
- **Always verify.** Never commit without a successful compile and test run.
- **Do NOT install anything.** All required tools (lizard, cmake, ctest, git) are already installed. Never run pip, apt, npm, or any package manager.
- **Do NOT use `touch` to force recompilation.** The build system tracks file modifications correctly.
- **Do NOT set `CCACHE_DISABLE=1` or any other env vars that disable caching.** Always run plain `ninja -C build-normal energyplus_tests` for incremental builds.
- **Do NOT use `sleep` commands.** Never wait/poll — just run the build or test command directly and wait for it to complete.
- **Do NOT run commands in the background.** Run builds and tests as foreground commands so you get the output directly.

## Final Summary

After completing all stages (or skipping ones that failed), report:
- Number of stages completed vs. skipped
- What each completed stage did (one line each)
- Approximate NLOC reduction (re-run lizard on the function to measure)
- Any stages that were skipped and why
