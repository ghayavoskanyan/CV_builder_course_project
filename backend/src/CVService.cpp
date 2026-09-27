#include "CVService.h"
#include "TemplateFactory.h"
#include "HTMLRenderStrategy.h"
#include "PDFRenderStrategy.h"
#include "JsonSerializer.h"
#include "Base64.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdio>

namespace {

std::string sanitizeFilename(const std::string& name) {
    std::string out;
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            out += c;
        } else if (c == ' ' || c == '-') {
            out += '_';
        }
    }
    if (out.empty()) {
        out = "cv";
    }
    return out;
}

std::string toLowerCopy(std::string value) {
    for (char& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value;
}

} // namespace

CVService::ExportResult CVService::exportDocument(
    const Document& document,
    const std::string& themeId,
    const std::string& format) const {

    ExportResult result;

    if (!TemplateFactory::isValidTheme(themeId)) {
        result.message = "Unknown theme: " + themeId;
        return result;
    }

    const std::string fmt = toLowerCopy(format);
    if (fmt != "html" && fmt != "pdf") {
        result.message = "Unknown format: " + format + ". Use 'html' or 'pdf'.";
        return result;
    }

    Document sorted = document;
    sorted.sortSectionsByOrder();

    const std::string baseName = sanitizeFilename(document.getFullName());

    if (fmt == "html") {
        auto strategy = std::make_shared<HTMLRenderStrategy>();
        auto templ = TemplateFactory::create(themeId, strategy);
        if (!templ) {
            result.message = "Failed to create template.";
            return result;
        }

        result.content = templ->renderToString(sorted);
        if (result.content.empty()) {
            result.message = "Export failed: document must have a name and at least one section.";
            return result;
        }

        result.filename = baseName + "_cv.html";
        result.mimeType = "text/html";
        result.success = true;
        return result;
    }

    auto strategy = std::make_shared<PDFRenderStrategy>();
    auto templ = TemplateFactory::create(themeId, strategy);
    if (!templ) {
        result.message = "Failed to create template.";
        return result;
    }

    const std::string markup = templ->renderToString(sorted);
    if (markup.empty()) {
        result.message = "Export failed: document must have a name and at least one section.";
        return result;
    }

    PDFRenderStrategy pdfStrategy;
    const std::string tempPath = "__cv_export_temp.pdf";
    if (!pdfStrategy.save(markup, tempPath)) {
        result.message = "PDF generation failed.";
        return result;
    }

    std::ifstream in(tempPath, std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        result.message = "Failed to read generated PDF.";
        return result;
    }
    std::string pdfBytes((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    in.close();
    std::remove(tempPath.c_str());

    result.contentBase64 = Base64::encode(pdfBytes);
    result.filename = baseName + "_cv.pdf";
    result.mimeType = "application/pdf";
    result.success = true;
    return result;
}

CVService::ExportResult CVService::previewDocument(
    const Document& document,
    const std::string& themeId) const {

    return exportDocument(document, themeId, "html");
}

std::string CVService::themesToJson() {
    std::ostringstream json;
    json << "{\"themes\":[";
    const auto themes = TemplateFactory::listThemes();
    for (std::size_t i = 0; i < themes.size(); ++i) {
        if (i > 0) {
            json << ',';
        }
        json << "{\"id\":\"" << JsonSerializer::escapeJsonString(themes[i].id) << "\""
             << ",\"name\":\"" << JsonSerializer::escapeJsonString(themes[i].name) << "\""
             << ",\"description\":\"" << JsonSerializer::escapeJsonString(themes[i].description) << "\""
             << ",\"layoutStyle\":\"" << JsonSerializer::escapeJsonString(themes[i].layoutStyle) << "\""
             << ",\"primaryColor\":\"" << JsonSerializer::escapeJsonString(themes[i].primaryColor) << "\""
             << "}";
    }
    json << "]}";
    return json.str();
}
