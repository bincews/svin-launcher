# SPDX-License-Identifier: GPL-3.0-only

import argparse
import hashlib
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description="Stamp a staged Svin portable package before archiving it.")
    parser.add_argument("directory", type=Path)
    package = parser.parse_args().directory
    marker = package / "portable.txt"
    if not marker.is_file() or not (package / "svinlauncher_updater.exe").is_file():
        parser.error("expected a staged portable Svin package")

    digest = hashlib.sha256()
    for file in sorted(path for path in package.rglob("*") if path.is_file() and path != marker):
        digest.update(file.relative_to(package).as_posix().encode("utf-8") + b"\0")
        with file.open("rb") as contents:
            digest.update(hashlib.file_digest(contents, "sha256").digest())
    label = "svin-payload-sha256:" + digest.hexdigest()
    marker.write_bytes(label.encode("ascii") + b"\n")
    print(label)


if __name__ == "__main__":
    main()
