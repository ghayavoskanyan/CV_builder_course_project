#ifndef RENDER_STRATEGY_H
#define RENDER_STRATEGY_H

#include <string>
#include "Document.h"

// Carries the visual/styling parameters that a Template computes and
// hands to a RenderStrategy. Keeping this as plain data (rather than
// letting the strategy know about Template subclasses) is what keeps
// the Strategy and Template Method hierarchies decoupled: a Template
// decides "what it should look like", a RenderStrategy decides "how
// to physically produce that in a given file format".
struct ThemeConfig {
    std::string themeName;
    std::string primaryColor;    // hex color, e.g. "#2C3E50"
    std::string secondaryColor;  // hex color
    std::string accentColor;     // hex color
    std::string fontFamily;      // CSS-style font stack
    std::string layoutStyle;     // "single-column" or "two-column"
};

// Strategy interface (Strategy Pattern) for converting a Document +
// ThemeConfig into a concrete output format (HTML, PDF, ...) and
// persisting it to disk. Concrete strategies are interchangeable at
// runtime and are selected by the user via CVEngine.
class RenderStrategy {
public:
    virtual ~RenderStrategy() = default;

    // Produces the rendered representation of the document as a
    // string. For text formats (HTML) this is the final output; for
    // binary formats (PDF) this may be an intermediate representation
    // that save() knows how to turn into real bytes.
    virtual std::string render(const Document& document, const ThemeConfig& theme) const = 0;

    // Persists previously rendered content to the given filepath.
    // Returns true on success, false on I/O failure.
    virtual bool save(const std::string& renderedContent, const std::string& filepath) const = 0;

    // File extension (including the leading dot) this strategy produces.
    virtual std::string getFileExtension() const = 0;

    // Human-readable name, used for logging/CLI feedback.
    virtual std::string getStrategyName() const = 0;
};

#endif // RENDER_STRATEGY_H
