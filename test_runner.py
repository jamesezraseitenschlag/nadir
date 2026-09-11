import glob
import subprocess
import os
import sys

def main():
    candidates = [
        'nadir',
        'nadir.exe',
        'build/nadir',
        'build/nadir.exe',
        'build/Release/nadir.exe',
        'build/Debug/nadir.exe',
        'bin/nadir',
        'bin/nadir.exe'
    ]
    exe_path = None
    for cand in candidates:
        full = os.path.abspath(cand)
        if os.path.exists(full) and not os.path.isdir(full):
            exe_path = full
            break

    if not exe_path:
        print("Error: Could not find nadir binary. Please build the project first using CMake or your C compiler.")
        sys.exit(1)

    files = glob.glob('test_repos/apex-recipes/**/*.cls', recursive=True) + glob.glob('test_repos/apex-recipes/**/*.trigger', recursive=True)
    if not files:
        files = glob.glob('examples/*.apex')

    passed = []
    failed = []

    for f in sorted(files):
        res = subprocess.run([exe_path, f], capture_output=True, text=True, errors='replace')
        if res.returncode == 0:
            passed.append(f)
        else:
            err = res.stderr.strip() or res.stdout.strip()
            failed.append((f, err))

    total = len(files)
    pct = (len(passed) / total * 100.0) if total > 0 else 0.0
    print(f"Total Test Files: {total}")
    print(f"Passed: {len(passed)} / {total} ({pct:.1f}%)")
    print(f"Failed: {len(failed)}")

    if failed:
        print("\nFailures:")
        for f, err in failed:
            print(f"[{os.path.basename(f)}] -> {err[:200]}")
        sys.exit(1)

if __name__ == '__main__':
    main()
