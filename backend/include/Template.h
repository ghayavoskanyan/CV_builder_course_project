#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <string>
#include <memory>
#include <iostream>
#include <utility>
#include "Document.h"
#include "RenderStrategy.h"

// Abstract base class implementing the Template Method pattern.
//
// Template defines the fixed skeleton of the "export a CV" algorithm
// in exportDocument(): validate -> pre-render hook -> build theme ->
// delegate actual rendering/saving to a RenderStrategy -> post-render
// hook. Concrete subclasses (ClassicTemplate, ModernTemplate) only
// customize the *varying* steps (their name and their ThemeConfig,
// plus optional extra logging), while the overall sequence of steps
// and the collaboration with RenderStrategy stays centralized here.
//
// Note the composition with RenderStrategy: the "style" (Template)
// and the "output format" (RenderStrategy) are two independent axes
// that can be mixed freely at runtime (e.g. Classic+HTML, Classic+PDF,
// Modern+HTML, Modern+PDF), which is exactly what CVEngine exposes
// to the user.
class Template {
public:
    explicit Template(std::shared_ptr<RenderStrategy> strategy)
        : strategy_(std::move(strategy)) {}

    virtual ~Template() = default;

    void setStrategy(std::shared_ptr<RenderStrategy> strategy) {
        strategy_ = std::move(strategy);
    }

    std::shared_ptr<RenderStrategy> getStrategy() const {
        return strategy_;
    }

    // Renders the document to an in-memory string without writing to disk.
    // Follows the same validate -> theme -> render pipeline as exportDocument.
    std::string renderToString(const Document& document) const {
        if (!strategy_) {
            return "";
        }
        if (!validateDocument(document)) {
            return "";
        }
        preRenderHook(document);
        ThemeConfig theme = buildThemeConfig();
        return strategy_->render(document, theme);
    }

    // ---- Template Method ----
    // The fixed algorithm skeleton. Intentionally not virtual: the
    // *sequence* of steps must never be altered by subclasses, only
    // the individual step implementations (via the protected hooks
    // below).
    bool exportDocument(const Document& document, const std::string& filepath) const {
        if (!strategy_) {
            std::cerr << "[Template] Error: no render strategy assigned to template '"
                      << getTemplateName() << "'.\n";
            return false;
        }

        if (!validateDocument(document)) {
            std::cerr << "[Template] Error: document failed validation for template '"
                      << getTemplateName() << "'.\n";
            return false;
        }

        preRenderHook(document);

        ThemeConfig theme = buildThemeConfig();
        std::string renderedContent = strategy_->render(document, theme);

        bool success = strategy_->save(renderedContent, filepath);

        postRenderHook(filepath, success);

        return success;
    }

    // Name of this concrete template style, e.g. "Classic", "Modern".
    virtual std::string getTemplateName() const = 0;

protected:
    // Varying step: each concrete template supplies its own colors,
    // fonts and layout preference.
    virtual ThemeConfig buildThemeConfig() const = 0;

    // Common validation step; subclasses may override to add stricter
    // rules (e.g. requiring specific sections to be present) while
    // still being able to call Template::validateDocument for the
    // baseline checks.
    virtual bool validateDocument(const Document& document) const {
        if (document.getFullName().empty()) {
            return false;
        }
        return !document.isEmpty();
    }

    // Hook executed before rendering begins. Default implementation
    // just logs; subclasses may override to add template-specific
    // notes while still invoking the base implementation.
    virtual void preRenderHook(const Document& document) const {
        std::cout << "[Template] Preparing '" << getTemplateName()
                  << "' layout for " << document.getFullName() << "...\n";
    }

    // Hook executed after the strategy has attempted to save the
    // output. Default implementation logs success/failure.
    virtual void postRenderHook(const std::string& filepath, bool success) const {
        if (success) {
            std::cout << "[Template] Successfully exported using '" << getTemplateName()
                      << "' template to: " << filepath << "\n";
        } else {
            std::cerr << "[Template] Failed to export document to: " << filepath << "\n";
        }
    }

    std::shared_ptr<RenderStrategy> strategy_;
};

#endif // TEMPLATE_H
