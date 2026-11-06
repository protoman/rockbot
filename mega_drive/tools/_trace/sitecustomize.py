"""Dependency recorder for tools/build_assets.py (loaded through PYTHONPATH only when
ASSETS_TRACE is set): logs every file a converter opens for READING and every directory it
lists, so the build can hash exactly those next time instead of a coarse input list."""
import atexit
import json
import os
import sys

_out = os.environ.get("ASSETS_TRACE")
if _out:
    _root = os.path.realpath(os.environ.get("ASSETS_ROOT", os.getcwd()))
    _top = os.path.dirname(_root)                 # the PC repository: sources and data live in both
    _files, _dirs, _written = set(), set(), set()

    def _keep(p):
        if not isinstance(p, str):
            return None
        p = os.path.realpath(p)
        if not p.startswith(_top + os.sep) or "__pycache__" in p or p.endswith(".pyc"):
            return None
        if "/tools/_trace/" in p or "/.git/" in p:
            return None
        return os.path.relpath(p, _root)

    def _hook(event, args):
        if event == "open":
            path, mode, flags = args
            rel = _keep(path)
            if isinstance(flags, int) and flags & 3:          # opened for writing: an output
                if rel:
                    _written.add(rel)       # never a dependency, even if it was read first
                return
            if rel and os.path.isfile(os.path.join(_root, rel)):
                _files.add(rel)
        elif event in ("os.listdir", "os.scandir"):
            rel = _keep(args[0] if args and args[0] not in (None, "") else ".")
            if rel and os.path.isdir(os.path.join(_root, rel)):
                _dirs.add(rel)

    sys.addaudithook(_hook)

    def _dump():
        with open(_out, "w") as f:
            json.dump({"files": sorted(_files - _written), "dirs": sorted(_dirs)}, f)

    atexit.register(_dump)
