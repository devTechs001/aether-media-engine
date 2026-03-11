# Python Build Script

#!/usr/bin/env python3
"""
AETHER Media Engine Build Script
=================================

Usage:
    python build.py [command] [options]

Commands:
    configure    Configure the build
    build        Build the project
    test         Run tests
    install      Install the project
    clean        Clean build artifacts
    all          Configure, build, and test

Options:
    --build-type TYPE    Build type (Debug/Release/RelWithDebInfo)
    --generator GEN      CMake generator (Ninja/Makefiles)
    --prefix PATH        Installation prefix
    --clean              Clean before building
    --help               Show this help
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path


def run_command(cmd, cwd=None):
    """Run a shell command."""
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, check=False)
    if result.returncode != 0:
        print(f"Command failed with exit code {result.returncode}")
        sys.exit(result.returncode)


def get_project_root():
    """Get the project root directory."""
    return Path(__file__).parent.absolute()


def configure(args):
    """Configure the build."""
    project_root = get_project_root()
    build_dir = project_root / "build"
    build_dir.mkdir(exist_ok=True)

    cmd = [
        "cmake",
        "-B", str(build_dir),
        f"-DCMAKE_BUILD_TYPE={args.build_type}",
        f"-G {args.generator}",
    ]

    if args.prefix:
        cmd.append(f"-DCMAKE_INSTALL_PREFIX={args.prefix}")

    run_command(cmd, cwd=project_root)


def build(args):
    """Build the project."""
    project_root = get_project_root()
    build_dir = project_root / "build"

    cmd = [
        "cmake",
        "--build", str(build_dir),
        "--config", args.build_type,
        "-j", str(os.cpu_count() or 4),
    ]

    run_command(cmd, cwd=project_root)


def test(args):
    """Run tests."""
    project_root = get_project_root()
    build_dir = project_root / "build"

    cmd = [
        "ctest",
        "--test-dir", str(build_dir),
        "--output-on-failure",
        "-j", str(os.cpu_count() or 4),
    ]

    run_command(cmd, cwd=project_root)


def install(args):
    """Install the project."""
    project_root = get_project_root()
    build_dir = project_root / "build"

    cmd = [
        "cmake",
        "--install", str(build_dir),
        "--prefix", args.prefix or "/usr/local",
    ]

    run_command(cmd, cwd=project_root)


def clean(args):
    """Clean build artifacts."""
    project_root = get_project_root()
    build_dir = project_root / "build"

    if build_dir.exists():
        print(f"Removing {build_dir}...")
        import shutil
        shutil.rmtree(build_dir)


def main():
    parser = argparse.ArgumentParser(description="AETHER Media Engine Build Script")
    parser.add_argument(
        "command",
        choices=["configure", "build", "test", "install", "clean", "all"],
        help="Command to run",
    )
    parser.add_argument(
        "--build-type",
        default="Release",
        choices=["Debug", "Release", "RelWithDebInfo"],
        help="Build type",
    )
    parser.add_argument(
        "--generator",
        default="Ninja",
        choices=["Ninja", "Unix Makefiles", "Visual Studio 17 2022"],
        help="CMake generator",
    )
    parser.add_argument(
        "--prefix",
        help="Installation prefix",
    )

    args = parser.parse_args()

    if args.command == "configure":
        configure(args)
    elif args.command == "build":
        build(args)
    elif args.command == "test":
        test(args)
    elif args.command == "install":
        install(args)
    elif args.command == "clean":
        clean(args)
    elif args.command == "all":
        clean(args)
        configure(args)
        build(args)
        test(args)


if __name__ == "__main__":
    main()
