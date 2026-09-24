#!/usr/bin/env python3
# Renders Documentation/MANUAL.md to a standalone Documentation/MANUAL.html -
# self-contained (inline CSS, no external requests), theme-aware (light/dark
# via prefers-color-scheme). Re-run this after any edit to MANUAL.md to keep
# the HTML in sync; the .html is generated output, not hand-edited.
#
# Usage:
#     pip install markdown
#     python3 scripts/build_manual_html.py

import pathlib
import markdown

ROOT = pathlib.Path(__file__).resolve().parent.parent
SRC = ROOT / "Documentation" / "MANUAL.md"
DST = ROOT / "Documentation" / "MANUAL.html"

TEMPLATE = """<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{title}</title>
<style>
  :root {{
    color-scheme: light dark;
    --bg: #ffffff;
    --fg: #1c1e21;
    --muted: #57606a;
    --border: #d0d7de;
    --code-bg: #f6f8fa;
    --link: #0969da;
    --table-stripe: #f6f8fa;
  }}
  @media (prefers-color-scheme: dark) {{
    :root {{
      --bg: #0d1117;
      --fg: #e6edf3;
      --muted: #8b949e;
      --border: #30363d;
      --code-bg: #161b22;
      --link: #4493f8;
      --table-stripe: #161b22;
    }}
  }}
  * {{ box-sizing: border-box; }}
  body {{
    margin: 0;
    background: var(--bg);
    color: var(--fg);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
    line-height: 1.6;
  }}
  main {{
    max-width: 860px;
    margin: 0 auto;
    padding: 2.5rem 1.5rem 5rem;
  }}
  h1, h2, h3 {{ line-height: 1.25; scroll-margin-top: 1rem; }}
  h1 {{ font-size: 2rem; border-bottom: 1px solid var(--border); padding-bottom: 0.5rem; }}
  h2 {{ font-size: 1.5rem; margin-top: 2.5rem; border-bottom: 1px solid var(--border); padding-bottom: 0.3rem; }}
  h3 {{ font-size: 1.15rem; margin-top: 1.75rem; }}
  a {{ color: var(--link); }}
  hr {{ border: none; border-top: 1px solid var(--border); margin: 2rem 0; }}
  code {{
    background: var(--code-bg);
    padding: 0.15em 0.4em;
    border-radius: 4px;
    font-size: 0.9em;
  }}
  pre {{
    background: var(--code-bg);
    padding: 1rem;
    border-radius: 6px;
    overflow-x: auto;
  }}
  pre code {{ padding: 0; background: none; }}
  table {{
    border-collapse: collapse;
    width: 100%;
    margin: 1rem 0;
    overflow-x: auto;
    display: block;
  }}
  th, td {{
    border: 1px solid var(--border);
    padding: 0.5rem 0.75rem;
    text-align: left;
    vertical-align: middle;
  }}
  tr:nth-child(even) {{ background: var(--table-stripe); }}
  ul, ol {{ padding-left: 1.5rem; }}
  li {{ margin: 0.25rem 0; }}
  img {{
    max-width: 100%;
    height: auto;
    image-rendering: pixelated;
    border: 1px solid var(--border);
    border-radius: 4px;
    margin: 0.25rem 0.5rem 0.25rem 0;
  }}
  em {{ color: var(--muted); }}
</style>
</head>
<body>
<main>
{body}
</main>
</body>
</html>
"""


def main():
    md_text = SRC.read_text(encoding="utf-8")
    body = markdown.markdown(
        md_text,
        extensions=["tables", "fenced_code", "toc"],
        extension_configs={"toc": {"anchorlink": False}},
    )
    title = md_text.splitlines()[0].lstrip("# ").strip()
    DST.write_text(TEMPLATE.format(title=title, body=body), encoding="utf-8")
    print(f"Wrote {DST.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
