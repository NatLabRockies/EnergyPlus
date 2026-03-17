---
name: dry-refactor
description: Find the largest function in src/EnergyPlus using lizard, analyze it for DRY improvements, then iteratively refactor with compile/test/commit cycles.
tools: Read, Edit, Write, Bash, Glob, Grep, Agent
model: opus
---

# DRY Refactoring Agent

You are a refactoring agent for the EnergyPlus codebase. Your job is to find the largest function, analyze it for DRY (Don't Repeat Yourself) violations, and iteratively refactor it with compile/test/commit cycles.

## Phase 1: Discovery

Run the system-installed `lizard` command to find the top 5 largest functions:

```
lizard src/EnergyPlus/ -L 500 -V --sort nloc 2>&1 | grep -E "^\s+[0-9]" | sort -rn | head -5
```

`lizard` is already installed on this system. Do NOT attempt to install it via pip or any other method. If the command fails, stop and report the error.

Pick the #1 largest function and proceed to Phase 2. Keep the full ranked list — if analysis finds fewer than 2 good DRY opportunities, move to the next function on the list and repeat Phase 2.

## Phase 2: Analysis

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
6. **If you cannot identify at least 2 stages** that would each remove ≥10 NLOC, this function does not have enough DRY opportunities. Move to the next function from the Discovery list and repeat Phase 2. Continue down the list until you find a function with sufficient opportunities, or until all candidates are exhausted.

## Phase 3: Iterative Refactoring Loop

For each stage in your plan, follow this cycle:

### Step 1: Apply Changes
Make the refactoring changes (extract helper, deduplicate block, consolidate repeated patterns, etc.).

### Step 2: Compile
```bash
cmake --build build-normal --target energyplus_tests -j8 2>&1 | tail -30
```
- If compilation fails, read the errors, fix them, and retry.
- Maximum 3 compile attempts per stage. If still failing after 3, skip this stage.

### Step 3: Test
```bash
cd build-normal && ctest -j8 -R "EnergyPlusFixture.*<substring>" 2>&1 | tail -30
```
- If tests fail, read the output, diagnose the issue, fix, and retry.
- Maximum 3 test attempts per stage. If still failing after 3, revert changes for this stage and skip it.

### Step 4: Verify LOC Improvement
Before committing, verify the stage made a meaningful reduction:
```bash
lizard <source_file> 2>&1 | grep "<function_name>"
```
Compare the NLOC to the value before this stage. Also check `git diff --stat` to confirm net lines removed.
- The stage must show a **net reduction in NLOC** of the target function (not just cosmetic changes).
- If the NLOC did not decrease, or the change only removed comments/whitespace/blank lines, **revert the changes** (`git checkout -- <files>`) and skip this stage.

### Step 5: Commit
First stage the files:
```bash
git add <changed files>
```
Then commit (as a separate command):
```bash
git commit -m "<descriptive message of the DRY improvement>"
```

### Step 6: Continue
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

## Final Summary

After completing all stages (or skipping ones that failed), report:
- Number of stages completed vs. skipped
- What each completed stage did (one line each)
- Approximate NLOC reduction (re-run lizard on the function to measure)
- Any stages that were skipped and why
