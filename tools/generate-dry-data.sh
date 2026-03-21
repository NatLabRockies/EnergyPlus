#!/usr/bin/env bash
# Generate dry-refactor-data.json from git history on dry-refactor-agent branch
# Usage: bash tools/generate-dry-data.sh [base_branch]

set -euo pipefail

BASE_BRANCH="${1:-develop}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT="$SCRIPT_DIR/dry-refactor-data.json"
REPO_ROOT="$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel)"

cd "$REPO_ROOT"

BRANCH=$(git rev-parse --abbrev-ref HEAD)
MERGE_BASE=$(git merge-base "$BASE_BRANCH" HEAD)

# Get all commit hashes in chronological order (oldest first)
mapfile -t HASHES < <(git log --reverse --format='%H' "$MERGE_BASE..HEAD")
TOTAL_COMMITS=${#HASHES[@]}

if [ "$TOTAL_COMMITS" -eq 0 ]; then
    echo '{"error": "No commits found"}' > "$OUTPUT"
    exit 1
fi

echo "Processing $TOTAL_COMMITS commits..." >&2

# Collect overall stats
read TOTAL_INS TOTAL_DEL < <(git diff --numstat "$MERGE_BASE..HEAD" | awk '{i+=$1; d+=$2} END {print i, d}')
NET_REDUCTION=$((TOTAL_DEL - TOTAL_INS))

# Start JSON output
{
    cat <<HEADER
{
  "branch": "$BRANCH",
  "baseBranch": "$BASE_BRANCH",
  "totalCommits": $TOTAL_COMMITS,
  "totalInsertions": $TOTAL_INS,
  "totalDeletions": $TOTAL_DEL,
  "netReduction": $NET_REDUCTION,
  "commits": [
HEADER

    # Per-commit data
    FIRST_COMMIT=1
    for i in "${!HASHES[@]}"; do
        HASH="${HASHES[$i]}"
        SHORT_HASH="${HASH:0:7}"
        TIMESTAMP=$(git log -1 --format='%ct' "$HASH")
        SUBJECT=$(git log -1 --format='%s' "$HASH" | sed 's/\\/\\\\/g; s/"/\\"/g; s/\t/\\t/g')

        if [ $FIRST_COMMIT -eq 1 ]; then
            FIRST_COMMIT=0
        else
            echo "    ,"
        fi

        echo "    {"
        echo "      \"hash\": \"$SHORT_HASH\","
        echo "      \"timestamp\": $TIMESTAMP,"
        echo "      \"subject\": \"$SUBJECT\","
        echo -n "      \"files\": ["

        # Per-file numstat for this commit
        FIRST_FILE=1
        while IFS=$'\t' read -r INS DEL FILEPATH; do
            [ -z "$FILEPATH" ] && continue
            # Skip binary files
            [ "$INS" = "-" ] && continue
            if [ $FIRST_FILE -eq 1 ]; then
                FIRST_FILE=0
                echo ""
            else
                echo ","
            fi
            echo -n "        {\"path\": \"$FILEPATH\", \"ins\": $INS, \"del\": $DEL}"
        done < <(git diff --numstat "${HASH}~1" "$HASH" 2>/dev/null || true)

        if [ $FIRST_FILE -eq 0 ]; then
            echo ""
            echo "      ]"
        else
            echo "]"
        fi
        echo -n "    }"

        # Progress
        if (( (i + 1) % 20 == 0 )); then
            echo "  Processed $((i+1))/$TOTAL_COMMITS commits..." >&2
        fi
    done

    echo ""
    echo "  ],"

    # Per-file cumulative sizes via running delta
    echo "  \"files\": {"

    # Get list of all files touched
    mapfile -t ALL_FILES < <(git diff --numstat "$MERGE_BASE..HEAD" | awk '{print $3}' | sort)
    TOTAL_FILES=${#ALL_FILES[@]}

    FIRST_FENTRY=1
    for FILE in "${ALL_FILES[@]}"; do
        [ -z "$FILE" ] && continue

        if [ $FIRST_FENTRY -eq 1 ]; then
            FIRST_FENTRY=0
        else
            echo "    ,"
        fi

        # Get baseline size (LOC at merge base)
        BASELINE_SIZE=$(git show "$MERGE_BASE:$FILE" 2>/dev/null | wc -l || true)
        BASELINE_SIZE=$(echo "$BASELINE_SIZE" | tr -dc '0-9')
        BASELINE_SIZE=${BASELINE_SIZE:-0}

        # Get current size
        CURRENT_SIZE=$(git show "HEAD:$FILE" 2>/dev/null | wc -l || true)
        CURRENT_SIZE=$(echo "$CURRENT_SIZE" | tr -dc '0-9')
        CURRENT_SIZE=${CURRENT_SIZE:-0}

        echo "    \"$FILE\": {"
        echo "      \"baselineSize\": $BASELINE_SIZE,"
        echo "      \"currentSize\": $CURRENT_SIZE,"
        echo -n "      \"snapshots\": ["

        # Build snapshots: track cumulative delta per commit that touches this file
        RUNNING_SIZE=$BASELINE_SIZE
        FIRST_SNAP=1
        echo ""
        echo -n "        {\"commitIdx\": -1, \"size\": $RUNNING_SIZE}"

        for i in "${!HASHES[@]}"; do
            HASH="${HASHES[$i]}"
            NUMSTAT=$(git diff --numstat "${HASH}~1" "$HASH" -- "$FILE" 2>/dev/null || true)
            [ -z "$NUMSTAT" ] && continue

            read INS DEL _ <<< "$NUMSTAT"
            [ "$INS" = "-" ] && continue
            RUNNING_SIZE=$((RUNNING_SIZE + INS - DEL))

            echo ","
            echo -n "        {\"commitIdx\": $i, \"size\": $RUNNING_SIZE}"
            FIRST_SNAP=0
        done

        echo ""
        echo "      ]"
        echo -n "    }"
    done

    echo ""
    echo "  },"

    # Function-level changes from diff hunk headers
    echo "  \"functions\": {"

    FIRST_FFILE=1
    for FILE in "${ALL_FILES[@]}"; do
        [ -z "$FILE" ] && continue
        # Only process C++ source files
        [[ "$FILE" != *.cc && "$FILE" != *.hh && "$FILE" != *.cpp && "$FILE" != *.hpp ]] && continue

        # Parse hunk headers to extract function names and line counts
        # Format: @@ -old,count +new,count @@ FunctionName
        FUNC_DATA=$(git log --reverse --format='COMMIT:%H' -p --diff-filter=M -U0 "$MERGE_BASE..HEAD" -- "$FILE" 2>/dev/null | awk '
        /^COMMIT:/ { commit_idx++ }
        /^@@/ {
            # Extract function context after the second @@
            n = index($0, "@@ ")
            rest = substr($0, n+3)
            n2 = index(rest, " @@")
            if (n2 > 0) {
                fname = substr(rest, n2+4)
                # Clean up: strip return type, take function name
                gsub(/^[ \t]+/, "", fname)
                gsub(/\(.*/, "", fname)
                gsub(/^.*[ *&]/, "", fname)
                if (fname != "" && fname !~ /^[{}]$/) {
                    # Parse hunk header for line counts
                    # @@ -start,count +start,count @@
                    split($0, parts, "@@")
                    hdr = parts[2]
                    gsub(/^[ \t]+/, "", hdr)
                    # Parse old range
                    split(hdr, ranges, " ")
                    old_range = ranges[1]  # -start,count
                    new_range = ranges[2]  # +start,count
                    if (index(old_range, ",") > 0) {
                        split(old_range, oc, ","); del = oc[2]+0
                    } else { del = 1 }
                    if (index(new_range, ",") > 0) {
                        split(new_range, nc, ","); ins = nc[2]+0
                    } else { ins = 1 }

                    func_ins[fname] += ins
                    func_del[fname] += del
                    key = fname SUBSEP commit_idx
                    if (!(key in seen)) {
                        seen[key] = 1
                        if (func_commits[fname] != "")
                            func_commits[fname] = func_commits[fname] "," (commit_idx - 1)
                        else
                            func_commits[fname] = (commit_idx - 1)
                    }
                }
            }
        }
        END {
            for (f in func_ins) {
                printf "%s\t%d\t%d\t%s\n", f, func_ins[f]+0, func_del[f]+0, func_commits[f]
            }
        }
        ' 2>/dev/null || true)

        [ -z "$FUNC_DATA" ] && continue

        if [ $FIRST_FFILE -eq 1 ]; then
            FIRST_FFILE=0
        else
            echo "    ,"
        fi

        echo "    \"$FILE\": ["
        FIRST_FUNC=1
        while IFS=$'\t' read -r FNAME FINS FDEL FCOMMITS; do
            [ -z "$FNAME" ] && continue
            if [ $FIRST_FUNC -eq 1 ]; then
                FIRST_FUNC=0
            else
                echo "      ,"
            fi
            # Escape function name
            FNAME_ESC=$(echo "$FNAME" | sed 's/\\/\\\\/g; s/"/\\"/g')
            echo -n "      {\"name\": \"$FNAME_ESC\", \"totalIns\": $FINS, \"totalDel\": $FDEL, \"commits\": [$FCOMMITS]}"
        done <<< "$FUNC_DATA"
        echo ""
        echo -n "    ]"
    done

    echo ""
    echo "  }"
    echo "}"

} > "$OUTPUT"

echo "Generated $OUTPUT ($TOTAL_COMMITS commits, $TOTAL_FILES files, net -$NET_REDUCTION lines)" >&2
