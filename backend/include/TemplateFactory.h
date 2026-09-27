#ifndef TEMPLATE_FACTORY_H
#define TEMPLATE_FACTORY_H

#include <memory>
#include <string>
#include <vector>
#include "Template.h"
#include "RenderStrategy.h"

struct ThemeInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string layoutStyle;
    std::string primaryColor;
};

// Factory Pattern: creates concrete Template instances by theme id
// without exposing subclass details to callers (CVService, ApiController).
class TemplateFactory {
public:
    static std::shared_ptr<Template> create(
        const std::string& themeId,
        std::shared_ptr<RenderStrategy> strategy);

    static std::vector<ThemeInfo> listThemes();
    static bool isValidTheme(const std::string& themeId);
};

#endif // TEMPLATE_FACTORY_H
