#include "ModernTemplate.h"
#include <iostream>
#include <utility>

ModernTemplate::ModernTemplate(std::shared_ptr<RenderStrategy> strategy)
    : Template(std::move(strategy)) {}

std::string ModernTemplate::getTemplateName() const {
    return "Modern";
}

ThemeConfig ModernTemplate::buildThemeConfig() const {
    ThemeConfig theme;
    theme.themeName = "Modern";
    theme.primaryColor = "#16A085";
    theme.secondaryColor = "#2C3E50";
    theme.accentColor = "#1ABC9C";
    theme.fontFamily = "'Helvetica Neue', Arial";
    theme.layoutStyle = "two-column";
    return theme;
}

void ModernTemplate::preRenderHook(const Document& document) const {
    Template::preRenderHook(document);
    std::cout << "[ModernTemplate] Applying clean sans-serif typography and vibrant accent styling.\n";
}
