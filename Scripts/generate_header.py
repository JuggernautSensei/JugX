import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

JUGX_ROOT = Path(__file__).resolve().parent.parent
JUGX_DIR = JUGX_ROOT / "JugX"
FILTERS_FILE = JUGX_ROOT / "JugX.vcxproj.filters"
PCH_FILE = JUGX_DIR / "pch.h"
OUTPUT_FILE = JUGX_DIR / "JugX.h"

MSBUILD_NS = "{http://schemas.microsoft.com/developer/msbuild/2003}"


def get_base_filter_headers() -> set[str]:
    root = ET.parse(FILTERS_FILE).getroot()
    headers = set()
    for cl_include in root.iter(f"{MSBUILD_NS}ClInclude"):
        filter_elem = cl_include.find(f"{MSBUILD_NS}Filter")
        if filter_elem is not None and filter_elem.text == "Base":
            name = Path(cl_include.attrib["Include"]).name
            headers.add(name)
    return headers


def get_pch_include_order() -> list[str]:
    text = PCH_FILE.read_text(encoding="utf-8")
    return re.findall(r'#include\s+"([^"]+)"', text)


def main() -> None:
    base_filter_headers = get_base_filter_headers()
    base_filter_headers.discard("pch.h")

    base_order = get_pch_include_order()
    if set(base_order) != base_filter_headers:
        missing_from_pch = base_filter_headers - set(base_order)
        missing_from_filter = set(base_order) - base_filter_headers
        if missing_from_pch:
            print(f"warning: in Base filter but not in pch.h: {sorted(missing_from_pch)}", file=sys.stderr)
        if missing_from_filter:
            print(f"warning: in pch.h but not tagged Base in filters: {sorted(missing_from_filter)}", file=sys.stderr)

    all_headers = {p.name for p in JUGX_DIR.glob("*.h")}
    all_headers.discard("pch.h")
    all_headers.discard("JugX.h")

    base_headers = [h for h in base_order if h in all_headers]
    rest_headers = sorted(all_headers - set(base_headers))

    lines = ["#pragma once", ""]
    lines.append("// ===========================================")
    lines.append("//  Base")
    lines.append("// ===========================================")
    lines.append("")
    lines += [f'#include "{h}"' for h in base_headers]
    lines.append("")
    lines.append("// ===========================================")
    lines.append("//  Headers")
    lines.append("// ===========================================")
    lines.append("")
    lines += [f'#include "{h}"' for h in rest_headers]
    lines.append("")

    OUTPUT_FILE.write_text("\n".join(lines), encoding="utf-8")
    print(f"wrote {OUTPUT_FILE} ({len(base_headers)} base + {len(rest_headers)} rest headers)")


if __name__ == "__main__":
    main()
