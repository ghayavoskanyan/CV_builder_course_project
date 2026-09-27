#ifndef MODERN_TEMPLATE_H
#define MODERN_TEMPLATE_H

#include "Template.h"

// Concrete Template implementing a clean, contemporary resume style:
// sans-serif typography, teal/emerald accent palette, two-column
// layout hint (used by the HTML strategy).
class ModernTemplate : public Template {
public:
    explicit ModernTemplate(std::shared_ptr<RenderStrategy> strategy);

    std::string getTemplateName() const override;

protected:
    ThemeConfig buildThemeConfig() const override;
    void preRenderHook(const Document& document) const override;
};

#endif // MODERN_TEMPLATE_H
