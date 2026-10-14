import argparse
import concurrent.futures
import itertools
import os
import signal
import subprocess
import sys
import threading
import time


PREVIEW_MAX_LINES = 24
PREVIEW_MAX_CHARS = 2400
PREVIEW_MAX_LINE_LENGTH = 300


def run_command(cmd, input_data=None, timeout=2.0):
    try:
        result = subprocess.run(
            cmd,
            input=input_data,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=timeout,
        )
        return result.returncode, result.stdout, result.stderr, False
    except subprocess.TimeoutExpired:
        return None, "", "TIME LIMIT EXCEEDED", True


def run_interactive(main_cmd, agent_cmd, input_data, timeout=2.0):
    main_process = subprocess.Popen(
        main_cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        bufsize=0,
    )
    agent_process = subprocess.Popen(
        agent_cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        bufsize=0,
    )

    main_to_agent = bytearray()
    agent_to_main = bytearray()
    main_stderr = bytearray()
    agent_stderr = bytearray()

    def relay(source, destination, transcript, prefix=b""):
        try:
            if prefix:
                destination.write(prefix)
                destination.flush()
            while True:
                data = source.read(4096)
                if not data:
                    break
                transcript.extend(data)
                destination.write(data)
                destination.flush()
        except (BrokenPipeError, OSError):
            pass
        finally:
            try:
                destination.close()
            except (BrokenPipeError, OSError):
                pass

    def collect(source, destination):
        try:
            while True:
                data = source.read(4096)
                if not data:
                    break
                destination.extend(data)
        except OSError:
            pass

    encoded_input = input_data.encode()
    threads = [
        threading.Thread(
            target=relay,
            args=(main_process.stdout, agent_process.stdin, main_to_agent),
            kwargs={"prefix": encoded_input},
            daemon=True,
        ),
        threading.Thread(
            target=relay,
            args=(agent_process.stdout, main_process.stdin, agent_to_main),
            kwargs={"prefix": encoded_input},
            daemon=True,
        ),
        threading.Thread(
            target=collect,
            args=(main_process.stderr, main_stderr),
            daemon=True,
        ),
        threading.Thread(
            target=collect,
            args=(agent_process.stderr, agent_stderr),
            daemon=True,
        ),
    ]
    for thread in threads:
        thread.start()

    deadline = time.monotonic() + timeout
    timed_out = False
    while main_process.poll() is None or agent_process.poll() is None:
        if time.monotonic() >= deadline:
            timed_out = True
            main_process.kill()
            agent_process.kill()
            break
        time.sleep(0.005)

    main_process.wait()
    agent_process.wait()
    for thread in threads:
        thread.join(timeout=0.5)

    decode = lambda data: bytes(data).decode(errors="replace")
    return {
        "main_returncode": main_process.returncode,
        "agent_returncode": agent_process.returncode,
        "main_to_agent": decode(main_to_agent),
        "agent_to_main": decode(agent_to_main),
        "main_stderr": decode(main_stderr),
        "agent_stderr": decode(agent_stderr),
        "timed_out": timed_out,
    }


def compile_cpp(src, exe, compiler, flags, force=False):
    if (
        not force
        and os.path.isfile(src)
        and os.path.isfile(exe)
        and os.path.getmtime(exe) >= os.path.getmtime(src)
    ):
        print(f"Build skipped (up to date): {src}")
        return

    print(f"Building: {src}")
    result = subprocess.run(
        [compiler, *flags, src, "-o", exe],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if result.returncode != 0:
        print(f"Compilation failed: {src}")
        print(result.stdout)
        print(result.stderr)
        raise SystemExit(1)


def compile_cpp_files(targets, compiler, flags, force=False):
    if len(targets) == 1:
        src, exe = targets[0]
        compile_cpp(src, exe, compiler, flags, force)
        return

    with concurrent.futures.ThreadPoolExecutor(
        max_workers=len(targets)
    ) as executor:
        futures = [
            executor.submit(
                compile_cpp,
                src,
                exe,
                compiler,
                flags,
                force,
            )
            for src, exe in targets
        ]
        for future in futures:
            future.result()


def normalize(text):
    # 末尾の空白・改行差だけを無視する。
    return text.strip()


def make_preview(text):
    if text == "":
        return "(empty)"

    preview_lines = []
    used_chars = 0
    omitted = False

    for line_number, line in enumerate(text.splitlines()):
        if line_number >= PREVIEW_MAX_LINES:
            omitted = True
            break

        if len(line) > PREVIEW_MAX_LINE_LENGTH:
            line = line[:PREVIEW_MAX_LINE_LENGTH] + " ... (line omitted)"
            omitted = True

        remaining = PREVIEW_MAX_CHARS - used_chars
        if remaining <= 0:
            omitted = True
            break
        if len(line) > remaining:
            preview_lines.append(line[:remaining] + " ... (omitted)")
            omitted = True
            break

        preview_lines.append(line)
        used_chars += len(line) + 1

    if omitted:
        preview_lines.append("... (remaining output omitted)")

    return "\n".join(preview_lines)


def print_block(label, text):
    print(label)
    print(make_preview(text))


def format_main_time(elapsed):
    return f"[main.cpp: {elapsed * 1000:.3f} ms]"


def save_case(
    input_data,
    out_main="",
    out_brute="",
    err_main="",
    err_brute="",
    save_brute=False,
):
    files = {
        "in.txt": input_data,
        "out_main.txt": out_main,
        "err_main.txt": err_main,
    }
    if save_brute:
        files["out_brute.txt"] = out_brute
        files["err_brute.txt"] = err_brute

    for filename, content in files.items():
        with open(filename, "w") as file:
            file.write(content)


def save_interactive_case(input_data, result):
    files = {
        "in.txt": input_data,
        "transcript_main_to_agent.txt": result["main_to_agent"],
        "transcript_agent_to_main.txt": result["agent_to_main"],
        "err_main.txt": result["main_stderr"],
        "err_agent.txt": result["agent_stderr"],
    }
    for filename, content in files.items():
        with open(filename, "w") as file:
            file.write(content)


def handle_interrupt(_signal_number, _frame):
    print("\nStopped by user.", file=sys.stderr)
    raise SystemExit(130)


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-n",
        "--tests",
        type=int,
        nargs="?",
        const=None,
        default=1000,
        help="number of tests; use -n without a value to run forever",
    )
    parser.add_argument("--start-seed", type=int, default=1)
    parser.add_argument("--compiler", type=str, default="g++")
    parser.add_argument("--std", type=str, default="c++20")
    parser.add_argument("--timeout", type=float, default=2.0)
    parser.add_argument("--no-compile", action="store_true")
    parser.add_argument(
        "--force-compile",
        action="store_true",
        help="rebuild all required C++ files even if they are up to date",
    )
    parser.add_argument(
        "--time",
        dest="show_time",
        action="store_true",
        help="show the wall-clock execution time of main.cpp for each test",
    )
    parser.add_argument(
        "--run-only",
        action="store_true",
        help="run main.cpp without compiling or running brute.cpp",
    )
    parser.add_argument(
        "--output-only",
        action="store_true",
        help="with --run-only, print only main.cpp stdout",
    )
    parser.add_argument(
        "--detail",
        action="store_true",
        help="also show the input and matching output for AC cases",
    )
    parser.add_argument(
        "--stop",
        action="store_true",
        help="stop immediately after the first WA",
    )
    parser.add_argument(
        "--interactive",
        action="store_true",
        help="connect main.cpp and agent.cpp for interactive testing",
    )
    parser.add_argument(
        "--manual",
        action="store_true",
        help=(
            "read one test case from stdin and compare main.cpp with brute.cpp"
        ),
    )

    args = parser.parse_args()
    if args.tests is not None and args.tests < 0:
        parser.error("-n/--tests must be non-negative")
    if args.no_compile and args.force_compile:
        parser.error("--no-compile and --force-compile cannot be used together")
    if args.interactive and args.run_only:
        parser.error("--interactive and --run-only cannot be used together")
    if args.manual and args.interactive:
        parser.error("--manual and --interactive cannot be used together")
    if args.manual and args.run_only:
        parser.error("--manual and --run-only cannot be used together")
    if args.output_only and not args.run_only:
        parser.error("--output-only is only available with --run-only")
    if args.interactive and args.show_time:
        parser.error(
            "--time is not available with --interactive because main.cpp "
            "waits for agent.cpp during the session"
        )
    return args


def main():
    signal.signal(signal.SIGINT, handle_interrupt)
    args = parse_args()

    flags = [
        f"-std={args.std}",
        "-O2",
        "-pipe",
        "-Wall",
        "-Wextra",
        "-Wshadow",
    ]

    main_src = "main.cpp"
    brute_src = "brute.cpp"
    agent_src = "agent.cpp"
    gen_src = "gen.cpp"

    main_exe = "./main"
    brute_exe = "./brute"
    agent_exe = "./agent"
    gen_exe = "./gen"

    if not args.no_compile:
        compile_targets = [(main_src, main_exe)]
        if args.interactive:
            compile_targets.extend([
                (gen_src, gen_exe),
                (agent_src, agent_exe),
            ])
        elif not args.run_only:
            compile_targets.append((brute_src, brute_exe))
            if not args.manual:
                compile_targets.append((gen_src, gen_exe))
        else:
            compile_targets.append((gen_src, gen_exe))

        compile_cpp_files(
            compile_targets,
            args.compiler,
            flags,
            force=args.force_compile,
        )

    if args.manual:
        if sys.stdin.isatty():
            print(
                "Enter one test case, then press Ctrl+D to run:",
                file=sys.stderr,
            )
        manual_input = sys.stdin.read()
        test_indices = range(1)
    else:
        manual_input = None
        test_indices = (
            itertools.count()
            if args.tests is None
            else range(args.tests)
        )
    wa_count = 0

    for test_index in test_indices:
        test_number = test_index + 1
        seed = args.start_seed + test_index
        test_label = "" if args.manual else f"{test_number} "
        case_description = (
            "manual input" if args.manual else f"seed = {seed}"
        )

        if args.manual:
            input_data = manual_input
        else:
            code, input_data, err, tle = run_command(
                [gen_exe, str(seed)],
                timeout=args.timeout,
            )
            if tle or code != 0:
                print("Generator failed")
                print(case_description)
                print(err)
                return

        if args.interactive:
            result = run_interactive(
                [main_exe],
                [agent_exe],
                input_data,
                timeout=args.timeout,
            )

            failure_reason = None
            if result["timed_out"]:
                failure_reason = "interactive session timed out"
            elif result["main_returncode"] != 0:
                failure_reason = (
                    f"main.cpp exited with code {result['main_returncode']}"
                )
            elif result["agent_returncode"] != 0:
                failure_reason = (
                    f"agent.cpp exited with code {result['agent_returncode']}"
                )
            elif result["main_stderr"] != "":
                failure_reason = "main.cpp wrote to stderr"

            if failure_reason is not None:
                print(f"{test_number} WA")
                print(f"[reason]\n{failure_reason}")
                print_block(
                    "[input for main.cpp and agent.cpp]",
                    input_data,
                )
                print_block(
                    "[agent.cpp -> main.cpp]",
                    result["agent_to_main"],
                )
                print_block(
                    "[main.cpp -> agent.cpp]",
                    result["main_to_agent"],
                )
                print_block("[main.cpp stderr]", result["main_stderr"])
                if result["agent_stderr"] != "":
                    print_block(
                        "[agent.cpp stderr]",
                        result["agent_stderr"],
                    )
                save_interactive_case(input_data, result)
                return

            print(f"{test_number} AC")
            if args.detail:
                print_block(
                    "[input for main.cpp and agent.cpp]",
                    input_data,
                )
                print_block(
                    "[agent.cpp -> main.cpp]",
                    result["agent_to_main"],
                )
                print_block(
                    "[main.cpp -> agent.cpp]",
                    result["main_to_agent"],
                )
                if result["agent_stderr"] != "":
                    print_block(
                        "[agent.cpp stderr]",
                        result["agent_stderr"],
                    )
            continue

        main_start = time.perf_counter()
        code_main, out_main, err_main, tle_main = run_command(
            [main_exe],
            input_data=input_data,
            timeout=args.timeout,
        )
        main_elapsed = time.perf_counter() - main_start
        main_time_text = (
            f" {format_main_time(main_elapsed)}"
            if args.show_time
            else ""
        )

        if tle_main:
            print(f"TLE in main.cpp{main_time_text}")
            print(case_description)
            save_case(input_data, out_main=out_main, err_main=err_main)
            return

        if code_main != 0:
            print(f"Runtime Error in main.cpp{main_time_text}")
            print(case_description)
            print(err_main)
            save_case(input_data, out_main=out_main, err_main=err_main)
            return

        if args.run_only:
            if args.output_only:
                print(
                    out_main,
                    end="" if out_main.endswith("\n") else "\n",
                )
                if args.show_time:
                    print(
                        f"test {test_number} "
                        f"{format_main_time(main_elapsed)}",
                        file=sys.stderr,
                    )
            else:
                print(
                    f"===== test {test_number}, seed = {seed}"
                    f"{main_time_text} ====="
                )
                print("[input]")
                print(
                    input_data,
                    end="" if input_data.endswith("\n") else "\n",
                )
                print("[output]")
                print(
                    out_main,
                    end="" if out_main.endswith("\n") else "\n",
                )
                print()
            continue

        code_brute, out_brute, err_brute, tle_brute = run_command(
            [brute_exe],
            input_data=input_data,
            timeout=args.timeout,
        )

        if tle_brute:
            print("TLE in brute.cpp")
            print(case_description)
            save_case(
                input_data,
                out_main,
                out_brute,
                err_main,
                err_brute,
                save_brute=True,
            )
            return

        if code_brute != 0:
            print("Runtime Error in brute.cpp")
            print(case_description)
            print(err_brute)
            save_case(
                input_data,
                out_main,
                out_brute,
                err_main,
                err_brute,
                save_brute=True,
            )
            return

        if normalize(out_main) != normalize(out_brute):
            wa_count += 1
            print(f"{test_label}WA{main_time_text}")
            print_block("[input]", input_data)
            print_block("[main.cpp output]", out_main)
            print_block("[brute.cpp output]", out_brute)
            save_case(
                input_data,
                out_main,
                out_brute,
                err_main,
                err_brute,
                save_brute=True,
            )
            if args.stop:
                return
            continue

        print(f"{test_label}AC{main_time_text}")
        if args.detail or args.manual:
            print_block("[input]", input_data)
            print_block("[matching output]", out_main)

    if args.run_only:
        print("Run-only finished!", file=sys.stderr)
    elif wa_count > 0:
        if args.manual:
            print("Finished: 1 test, 1 WA")
        else:
            print(f"Finished: {args.tests} tests, {wa_count} WA")
    else:
        print("All tests passed!")


if __name__ == "__main__":
    main()
