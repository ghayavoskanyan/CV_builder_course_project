# Template CV Builder

An interactive C++17 command-line application that lets a user fill in
the dynamic sections of a resume (personal info, work experience,
education, skills, projects), pick a visual **template style**, pick
an **export format**, and compile a ready-made CV to disk as a styled
`.html` file or a genuine, standards-compliant `.pdf` file — with no
third-party libraries required.

## Design Patterns

| Pattern             | Where                                                         | Why                                                                                                   |
|----------------------|----------------------------------------------------------------|---------------------------------------------------------------------------------------------------------|
| **Strategy**         | `RenderStrategy`, `HTMLRenderStrategy`, `PDFRenderStrategy`     | Swaps the *output format* (HTML vs. PDF) independently of everything else, at runtime.                  |
| **Template Method**  | `Template`, `ClassicTemplate`, `ModernTemplate`                 | Fixes the *export algorithm skeleton* (validate → theme → render → save → log) while letting subclasses vary only the style (colors, fonts, layout).|
| **Composition**      | `Template` holds a `shared_ptr<RenderStrategy>`                 | Style and format are two independent axes — any template can be combined with any strategy (4 combinations from 2×2 classes). |
| **Controller (MVC-ish)** | `CVEngine`                                                  | Owns the interactive CLI loop, assembles the `Document`, and triggers export — contains no rendering logic itself. |

## Project Structure

```
cv_builder/
├── include/
│   ├── Section.h              # dynamic CV section (fields + repeating entries)
│   ├── Document.h              # aggregates Sections + candidate identity
│   ├── RenderStrategy.h        # Strategy interface + ThemeConfig
│   ├── HTMLRenderStrategy.h    # concrete Strategy: styled HTML5 output
│   ├── PDFRenderStrategy.h     # concrete Strategy: hand-rolled PDF writer
│   ├── Template.h              # Template Method skeleton (header-only)
│   ├── ClassicTemplate.h       # serif / single-column style
│   ├── ModernTemplate.h        # sans-serif / two-column-accent style
│   └── CVEngine.h              # CLI controller
├── src/
│   ├── Section.cpp
│   ├── Document.cpp
│   ├── HTMLRenderStrategy.cpp
│   ├── PDFRenderStrategy.cpp
│   ├── ClassicTemplate.cpp
│   ├── ModernTemplate.cpp
│   └── CVEngine.cpp
├── main.cpp
├── Makefile
└── README.md
```

## Building

Requires a C++17 compiler (g++ ≥ 7 or clang++ ≥ 5) and `make`.

```bash
make            # builds ./cv_builder
make run        # builds and immediately runs it
make clean      # removes build artifacts and any generated .html/.pdf files
```

## Running

```bash
./cv_builder
```

You'll walk through a menu:

1. Enter/Edit Personal Information
2. Enter/Edit Professional Summary
3. Add Work Experience (repeatable entries)
4. Add Education (repeatable entries)
5. Add Skills
6. Add Projects (optional, repeatable entries)
7. Choose Template & Format — pick **Classic** or **Modern**, and **HTML** or **PDF**
8. Preview Document — quick summary of what's been entered so far
9. Export CV — writes `<your filename>.html` or `<your filename>.pdf` to the current directory
0. Exit

You can revisit any menu option before exporting, and you can export
multiple times with different template/format choices without losing
your entered data.

## Notable Implementation Details

- **`Section`** models both flat key/value data (Personal Info,
  Skills) and repeating multi-field records (Work Experience,
  Education, Projects) with one class, keyed by a `SectionType` enum
  and an `order` used for consistent section ordering on export.
- **`PDFRenderStrategy`** writes real PDF bytes by hand: PDF objects,
  a content stream per page with `BT/Tf/Td/Tj/ET` operators, an
  accurate `xref` table with byte offsets, and a `trailer` — validated
  in development with `qpdf --check` and `pdftotext`. It performs
  genuine word-wrapping using the standard Helvetica AFM glyph-width
  table so long descriptions wrap to new lines instead of running off
  the page, and it automatically paginates onto additional pages when
  content overflows a single Letter-sized page.
- **`HTMLRenderStrategy`** emits a single self-contained HTML5 file
  with embedded CSS (no external assets), HTML-escaping all user
  input, and adapting its layout (`single-column` vs `two-column`)
  based on the `ThemeConfig` supplied by the active `Template`.
- **`Template::exportDocument`** is the non-virtual Template Method:
  it always validates → runs a pre-render hook → builds a
  `ThemeConfig` (virtual, subclass-specific) → delegates to the
  `RenderStrategy` → runs a post-render hook. Subclasses only ever
  override the *varying* steps, never the sequence itself.
- All shared ownership (`Section`s inside a `Document`, the active
  `RenderStrategy` inside a `Template`) uses `std::shared_ptr`; no raw
  owning pointers appear anywhere in the codebase.
