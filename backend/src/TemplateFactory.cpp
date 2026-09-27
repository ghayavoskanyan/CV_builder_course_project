#include "TemplateFactory.h"
#include "ClassicTemplate.h"
#include "ModernTemplate.h"
#include "ElegantTemplate.h"
#include <algorithm>
#include <cctype>

namespace {

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

} // namespace

std::shared_ptr<Template> TemplateFactory::create(
    const std::string& themeId,
    std::shared_ptr<RenderStrategy> strategy) {

    const std::string id = toLower(themeId);
    if (id == "classic") {
        return std::make_shared<ClassicTemplate>(strategy);
    }
    if (id == "modern") {
        return std::make_shared<ModernTemplate>(strategy);
    }
    if (id == "elegant") {
        return std::make_shared<ElegantTemplate>(strategy);
    }
    return nullptr;
}

std::vector<ThemeInfo> TemplateFactory::listThemes() {
    return {
        {"classic", "Classic", "Traditional formal style with serif typography and navy palette",
         "single-column", "#2C3E50"},
        {"modern", "Modern", "Clean contemporary style with sans-serif typography and teal accents",
         "two-column", "#16A085"},
        {"elegant", "Elegant", "Refined style with serif typography and burgundy/gold palette",
         "single-column", "#4A1942"}
    };
}

bool TemplateFactory::isValidTheme(const std::string& themeId) {
    const std::string id = toLower(themeId);
    return id == "classic" || id == "modern" || id == "elegant";
}
