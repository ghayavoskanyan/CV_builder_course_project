#include "ElegantTemplate.h"
#include <iostream>
#include <utility>

ElegantTemplate::ElegantTemplate(std::shared_ptr<RenderStrategy> strategy)
    : Template(std::move(strategy)) {}

std::string ElegantTemplate::getTemplateName() const {
    return "Elegant";
}

ThemeConfig ElegantTemplate::buildThemeConfig() const {
    ThemeConfig theme;
    theme.themeName = "Elegant";
    theme.primaryColor = "#4A1942";
    theme.secondaryColor = "#7D5A72";
    theme.accentColor = "#C9A227";
    theme.fontFamily = "'Palatino Linotype', 'Book Antiqua', Georgia";
    theme.layoutStyle = "single-column";
    return theme;
}

void ElegantTemplate::preRenderHook(const Document& document) const {
    Template::preRenderHook(document);
    std::cout << "[ElegantTemplate] Applying refined serif typography with burgundy and gold accents.\n";
}
