#ifndef HTML_RENDER_STRATEGY_H
#define HTML_RENDER_STRATEGY_H

#include <memory>
#include "RenderStrategy.h"
#include "Section.h"

// Concrete Strategy that renders a Document as a single, styled,
// self-contained HTML5 file (CSS is embedded, no external assets).
class HTMLRenderStrategy : public RenderStrategy {
public:
    std::string render(const Document& document, const ThemeConfig& theme) const override;
    bool save(const std::string& renderedContent, const std::string& filepath) const override;
    std::string getFileExtension() const override;
    std::string getStrategyName() const override;

private:
    static std::string escapeHtml(const std::string& input);
    std::string buildCss(const ThemeConfig& theme) const;
    std::string renderSection(const std::shared_ptr<Section>& section) const;
};

#endif // HTML_RENDER_STRATEGY_H
