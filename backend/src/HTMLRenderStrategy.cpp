#include "HTMLRenderStrategy.h"
#include <fstream>
#include <sstream>

std::string HTMLRenderStrategy::escapeHtml(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&#39;";  break;
            default:   out += c;        break;
        }
    }
    return out;
}

std::string HTMLRenderStrategy::buildCss(const ThemeConfig& theme) const {
    std::ostringstream css;
    css << "    body { font-family: " << theme.fontFamily << ", sans-serif; "
        << "margin: 0; padding: 0; background-color: #f2f2f2; color: #222; }\n";
    css << "    .cv-container { max-width: 850px; margin: 30px auto; background: #ffffff; "
        << "box-shadow: 0 0 15px rgba(0,0,0,0.12); padding: 40px 44px; }\n";
    css << "    .cv-header { border-bottom: 3px solid " << theme.primaryColor
        << "; padding-bottom: 14px; margin-bottom: 22px; display: flex; "
        << "align-items: center; gap: 24px; }\n";
    css << "    .cv-header-text { flex: 1; min-width: 0; }\n";
    css << "    .cv-photo { width: 110px; height: 110px; border-radius: 50%; "
        << "object-fit: cover; border: 3px solid " << theme.accentColor
        << "; flex-shrink: 0; }\n";
    css << "    .cv-header h1 { margin: 0; color: " << theme.primaryColor
        << "; font-size: 2.2em; }\n";
    css << "    .cv-header h2 { margin: 4px 0 0 0; color: " << theme.secondaryColor
        << "; font-weight: 400; font-size: 1.15em; }\n";
    css << "    .cv-contact { color: " << theme.secondaryColor
        << "; font-size: 0.92em; margin: 10px 0 0 0; }\n";
    css << "    .cv-section { margin-bottom: 24px; }\n";
    css << "    .cv-section h3 { color: #ffffff; background-color: " << theme.primaryColor
        << "; display: inline-block; padding: 5px 14px; border-radius: 4px; "
        << "text-transform: uppercase; letter-spacing: 1px; font-size: 0.95em; margin: 0 0 12px 0; }\n";
    css << "    .cv-field { margin: 4px 0; }\n";
    css << "    .cv-field .label { font-weight: 600; color: " << theme.secondaryColor
        << "; margin-right: 6px; }\n";
    css << "    .cv-entry { border-left: 3px solid " << theme.accentColor
        << "; padding-left: 14px; margin-bottom: 14px; }\n";
    css << "    .cv-entry .entry-line { margin: 2px 0; }\n";
    css << "    .cv-entry .entry-line .label { font-weight: 600; color: " << theme.secondaryColor
        << "; }\n";
    if (theme.layoutStyle == "two-column") {
        css << "    .cv-body { display: flex; gap: 34px; }\n";
        css << "    .cv-main { flex: 2; min-width: 0; }\n";
    } else {
        css << "    .cv-body { display: block; }\n";
        css << "    .cv-main { width: 100%; }\n";
    }
    return css.str();
}

std::string HTMLRenderStrategy::renderSection(const std::shared_ptr<Section>& section) const {
    std::ostringstream html;
    html << "    <div class=\"cv-section\">\n";
    html << "      <h3>" << escapeHtml(section->getTitle()) << "</h3>\n";

    for (const auto& field : section->getFields()) {
        html << "      <div class=\"cv-field\">";
        if (!field.first.empty()) {
            html << "<span class=\"label\">" << escapeHtml(field.first) << ":</span>";
        }
        html << "<span class=\"value\">" << escapeHtml(field.second) << "</span>";
        html << "</div>\n";
    }

    for (const auto& entry : section->getEntries()) {
        html << "      <div class=\"cv-entry\">\n";
        for (const auto& kv : entry) {
            html << "        <div class=\"entry-line\">";
            if (!kv.first.empty()) {
                html << "<span class=\"label\">" << escapeHtml(kv.first) << ":</span> ";
            }
            html << "<span class=\"value\">" << escapeHtml(kv.second) << "</span>";
            html << "</div>\n";
        }
        html << "      </div>\n";
    }

    html << "    </div>\n";
    return html.str();
}

std::string HTMLRenderStrategy::render(const Document& document, const ThemeConfig& theme) const {
    std::ostringstream html;
    html << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
    html << "  <meta charset=\"UTF-8\">\n";
    html << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html << "  <title>" << escapeHtml(document.getFullName()) << " - CV</title>\n";
    html << "  <style>\n" << buildCss(theme) << "  </style>\n";
    html << "</head>\n<body>\n";
    html << "  <div class=\"cv-container\">\n";
    html << "    <div class=\"cv-header\">\n";
    if (document.hasPhoto()) {
        html << "      <img class=\"cv-photo\" src=\"data:" << document.getPhotoMimeType()
             << ";base64," << document.getPhotoBase64() << "\" alt=\"Profile photo\">\n";
    }
    html << "      <div class=\"cv-header-text\">\n";
    html << "        <h1>" << escapeHtml(document.getFullName()) << "</h1>\n";
    if (!document.getJobTitle().empty()) {
        html << "        <h2>" << escapeHtml(document.getJobTitle()) << "</h2>\n";
    }

    auto personalInfo = document.findSection(SectionType::PERSONAL_INFO);
    if (personalInfo && !personalInfo->getFields().empty()) {
        html << "        <p class=\"cv-contact\">";
        const auto& fields = personalInfo->getFields();
        for (std::size_t i = 0; i < fields.size(); ++i) {
            html << escapeHtml(fields[i].second);
            if (i + 1 < fields.size()) {
                html << " &nbsp;|&nbsp; ";
            }
        }
        html << "</p>\n";
    }
    html << "      </div>\n";
    html << "    </div>\n";
    html << "    <div class=\"cv-body\">\n";
    html << "      <div class=\"cv-main\">\n";

    for (const auto& section : document.getSections()) {
        if (section && section->getType() != SectionType::PERSONAL_INFO && !section->isEmpty()) {
            html << renderSection(section);
        }
    }

    html << "      </div>\n";   // cv-main
    html << "    </div>\n";     // cv-body
    html << "  </div>\n";       // cv-container
    html << "</body>\n</html>\n";
    return html.str();
}

bool HTMLRenderStrategy::save(const std::string& renderedContent, const std::string& filepath) const {
    std::ofstream out(filepath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }
    out << renderedContent;
    bool ok = out.good();
    out.close();
    return ok;
}

std::string HTMLRenderStrategy::getFileExtension() const {
    return ".html";
}

std::string HTMLRenderStrategy::getStrategyName() const {
    return "HTML Render Strategy";
}
