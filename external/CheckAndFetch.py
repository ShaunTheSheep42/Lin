#!/usr/bin/env python3

# This script checks for the existence of each target resource (LIBS)
# if a resource is missing, it uses Git to fetch it from
# the specified (TARGET) location.
#
# Note: The check is coarse-grained, verifying only
# the existence of a directory with the same name (NAME).

import os
import shutil
import subprocess

TempDir = "./Temp"

LIBS = [
    {
        "NAME": "magic_enum",
        "VERSION": "v0.9.8",
        "TARGET": "https://github.com/Neargye/magic_enum.git",
        "KEEP": [
            "include/magic_enum",
            "LICENSE",
        ],
    },
    {
        "NAME": "spdlog",
        "VERSION": "v1.17.0",
        "TARGET": "https://github.com/gabime/spdlog.git",
        "KEEP": [
            "include/spdlog",
            "LICENSE",
        ],
    },
    {
        "NAME": "glaze",
        "VERSION": "v9.0.0",
        "TARGET": "https://github.com/stephenberry/glaze.git",
        "KEEP": [
            "include/glaze",
            "LICENSE",
        ],
    },
    {
        "NAME": "catch2",
        "VERSION": "v3.6.0",
        "TARGET": "https://github.com/catchorg/Catch2.git",
        "KEEP": [
            "src",
            "extras",
            "CMake",
            "CMakeLists.txt",
            "LICENSE.txt",
        ],
    },
]

os.makedirs(TempDir, exist_ok=True)

for lib in LIBS:
    name = lib["NAME"]
    version = lib["VERSION"]
    github = lib["TARGET"]
    keep_paths = lib["KEEP"]

    dest = os.path.join(TempDir, name)
    keep_dest = os.path.join("./", name)

    if os.path.exists(keep_dest):
        print(f"Lib exist, skip: {keep_dest}")
        continue
    else:
        print(f"Cloning {name} ({version}) from {github} ...", flush=True)
        try:
            result = subprocess.run(
                ["git", "clone", "--depth", "1", "--branch", version, github, dest],
                check=True,
                capture_output=True,
                text=True,
            )
            if result.stdout.strip():
                print(result.stdout.strip())
        except subprocess.CalledProcessError as e:
            print(f"[ERROR] Failed to clone {name} ({version})")
            print(f"Command: {e.cmd}")
            print(f"Exit code: {e.returncode}")
            if e.stderr:
                print(f"Git error: {e.stderr.strip().splitlines()[-1]}")
            exit(1)

    os.makedirs(keep_dest, exist_ok=True)

    for keep_path in keep_paths:
        keep_src = os.path.join(dest, keep_path)
        if os.path.exists(keep_src):
            print(f"Copying {keep_src} -> {keep_dest}")
            if os.path.isdir(keep_src):
                shutil.copytree(
                    keep_src,
                    os.path.join(keep_dest, os.path.basename(keep_src)),
                    dirs_exist_ok=True,
                )
            else:
                shutil.copy2(keep_src, keep_dest)
        else:
            print(f"Warning: {keep_src} not found, skip.")

if os.path.exists(TempDir):
    print(f"Removing temporary directory: {TempDir}")
    shutil.rmtree(TempDir)
