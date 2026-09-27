#include "ClassicTemplate.h"
#include <iostream>
#include <utility>

ClassicTemplate::ClassicTemplate(std::shared_ptr<RenderStrategy> strategy)
    : Template(std::move(strategy)) {}

std::string ClassicTemplate::getTemplateName() const {
    return "Classic";
}

ThemeConfig ClassicTemplate::buildThemeConfig() const {
    ThemeConfig theme;
    theme.themeName = "Classic";
    theme.primaryColor = "#2C3E50";
    theme.secondaryColor = "#34495E";
    theme.accentColor = "#7F8C8D";
    theme.fontFamily = "Georgia, 'Times New Roman'";
    theme.layoutStyle = "single-column";
    return theme;
}

void ClassicTemplate::preRenderHook(const Document& document) const {
    Template::preRenderHook(document);
    std::cout << "[ClassicTemplate] Applying traditional serif typography and formal single-column layout.\n";
}
