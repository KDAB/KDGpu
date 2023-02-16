#pragma once

namespace ToyRenderer {

struct PresentOptions;

struct ApiQueue {
    // TODO: Return type and arguments?
    virtual void submit() = 0;
    virtual void present(const PresentOptions &options) = 0;
};

} // namespace ToyRenderer
