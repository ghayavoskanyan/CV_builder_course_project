#ifndef PDF_RENDER_STRATEGY_H
#define PDF_RENDER_STRATEGY_H

#include <string>
#include <vector>
#include "RenderStrategy.h"

// Concrete Strategy that renders a Document into a genuine,
// standards-compliant PDF file, built by hand from raw PDF object
// syntax (no third-party PDF library dependency, per the project's
// standard-library-only constraint).
//
// render() produces a lightweight intermediate "markup" string (a
// simple line-oriented tagged format) describing the document's
// content in a format-agnostic way. save() is responsible for the
// format-specific work: parsing that markup, laying it out across
// one or more Letter-sized pages, and emitting valid PDF bytes
// (objects, content streams, xref table, trailer).
class PDFRenderStrategy : public RenderStrategy {
public:
    std::string render(const Document& document, const ThemeConfig& theme) const override;
    bool save(const std::string& renderedContent, const std::string& filepath) const override;
    std::string getFileExtension() const override;
    std::string getStrategyName() const override;

private:
    // One instruction parsed out of the intermediate markup produced
    // by render(), e.g. { tag = "FIELD", label = "Email", value = "a@b.com" }.
    struct MarkupLine {
        std::string tag;
        std::string label;
        std::string value;
    };

    std::vector<MarkupLine> parseMarkup(const std::string& markup) const;
    static std::string escapePdfText(const std::string& input);
    std::string buildPdfDocument(const std::vector<MarkupLine>& lines) const;
};

#endif // PDF_RENDER_STRATEGY_H
