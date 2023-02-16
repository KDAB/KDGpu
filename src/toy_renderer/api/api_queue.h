#pragma once

namespace ToyRenderer {

struct PresentOptions;
struct SubmitOptions;

struct ApiQueue {
    // TODO: Return type and arguments?
    virtual void submit(const SubmitOptions &options) = 0;
    virtual void present(const PresentOptions &options) = 0;
};

} // namespace ToyRenderer
