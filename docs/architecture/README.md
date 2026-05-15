# Architecture Map

Interactive single-page reference for the Crossfire Client codebase.

| File | Description |
|---|---|
| `index.html` | Self-contained interactive architecture map (open in any browser) |
| `architecture.json` | Machine-readable data: layers, files, data flows, dependencies, design decisions |

## Viewing

Open `index.html` directly in a browser — no server required, no build step.
The HTML page fetches `architecture.json` via a relative path; both files must
stay in the same directory.

On GitHub, the rendered view is available via GitHub Pages if enabled for this
repository.

## Contents

The architecture map covers five tabs:

- **Overview** — layered architecture diagram with clickable layer rows
- **Layers & Files** — every source file with its role and line count; searchable
- **Data Flow** — runtime data and control paths from server to screen
- **Dependencies** — all external libraries with required/optional status
- **Design Decisions** — rationale behind key GTK2→GTK3 port choices
