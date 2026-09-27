#ifndef ELEGANT_TEMPLATE_H
#define ELEGANT_TEMPLATE_H

#include "Template.h"

// Concrete Template implementing an elegant, refined resume style:
// serif typography, deep burgundy/gold palette, single-column layout.
class ElegantTemplate : public Template {
public:
    explicit ElegantTemplate(std::shared_ptr<RenderStrategy> strategy);

    std::string getTemplateName() const override;

protected:
    ThemeConfig buildThemeConfig() const override;
    void preRenderHook(const Document& document) const override;
};

#endif // ELEGANT_TEMPLATE_H
