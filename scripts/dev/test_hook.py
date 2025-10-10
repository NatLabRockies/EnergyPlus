from pathlib import Path

from base_hook import (
    SRC_DIR,
    ErrorMessage,
    LogLevel,
    LogMessage,
    exit_hook,
    flatten_list_of_lists,
    get_base_parser,
    parallel_apply,
    report_log_messages,
)


def function_that_issues_several_errors(filepath: Path, is_error: bool = False) -> list[LogMessage]:
    errors: list[LogMessage] = []
    if is_error:
        errors.append(
            ErrorMessage(
                tool="test_hook",
                filepath=filepath,
                line_number=10,
                line="This is a dummy line content",
                message="I'm issuing an error",
            )
        )
        errors.append(
            ErrorMessage(
                tool="test_hook",
                filepath=filepath,
                line_number=250,
                line="This is another dummy line content",
                message="I'm issuing a second error",
            )
        )
    return errors


def function_that_issues_one_error(filepath: Path, is_error: bool = False) -> LogMessage | None:
    if is_error:
        return ErrorMessage(
            tool="test_hook",
            filepath=filepath,
            line_number=10,
            line="This is a dummy line content",
            message="I'm issuing a unique error",
        )
    return None


if __name__ == "__main__":
    parser = get_base_parser(description="Test Hooks")
    parser.add_argument("--single-error", action="store_true", help="Issue a single error")
    parser.add_argument("--gha", action="store_true", help="Fake Being on Github Actions")
    parser.add_argument("--debug", action="store_true", help="Print files and exit")
    args = parser.parse_args()

    if args.gha:
        import os
        import tempfile

        os.environ["GITHUB_ACTIONS"] = "true"
        step_summary = Path(tempfile.mkdtemp()) / "step_summary.md"
        os.environ["GITHUB_STEP_SUMMARY"] = str(step_summary)

    exts = {".cc", ".hh"}
    exts = None
    if len(args.files) > 0:
        n_ori = len(args.files)
        if exts is None:
            files = args.files
        else:
            files = [f for f in args.files if f.suffix in exts]
        if args.verbose:
            print(f"Checking {len(files)} of {n_ori} specified files")
    else:
        files = []
        if exts is None:
            exts = {".*"}
        for e in exts:
            files += list(SRC_DIR.glob(f"**/*{e}"))
        if args.verbose:
            print(f"Checking {len(files)} files")
    if len(files) == 0:
        print("No files to check")
        exit(0)

    if args.debug:
        print(files)
        exit(0)

    if args.single_error:
        log_messages = [function_that_issues_one_error(filepath=files[0], is_error=True)]
        log_messages += parallel_apply(func=function_that_issues_one_error, filepaths=files[1:], is_error=False)
        success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
        if args.gha:
            print("\n====== content of GITHUB_STEP_SUMMARY ======")
            print(step_summary.read_text())
            print("============================================")
        exit_hook(success=success)
    else:
        errors_list_of_lists = [function_that_issues_several_errors(filepath=files[0], is_error=True)]
        errors_list_of_lists += parallel_apply(
            func=function_that_issues_several_errors, filepaths=files[1:], is_error=False
        )
        log_messages = flatten_list_of_lists(list_of_lists=errors_list_of_lists)
        success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)

        if args.gha:
            print("\n====== content of GITHUB_STEP_SUMMARY ======")
            print(step_summary.read_text())
            print("============================================")

        exit_hook(success=success)
