#ifndef CLASSIC_TEMPLATE_H
#define CLASSIC_TEMPLATE_H

#include "Template.h"

// Concrete Template implementing a traditional, formal resume style:
// serif typography, muted navy/slate palette, single-column layout.
class ClassicTemplate : public Template {
public:
    explicit ClassicTemplate(std::shared_ptr<RenderStrategy> strategy);

    std::string getTemplateName() const override;

protected:
    ThemeConfig buildThemeConfig() const override;
    void preRenderHook(const Document& document) const override;
};

#endif // CLASSIC_TEMPLATE_H
