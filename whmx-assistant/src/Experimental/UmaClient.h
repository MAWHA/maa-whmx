#pragma once

#include <QtWidgets/QWidget>
#include <gsl/gsl>

namespace Experimental {

class UmaClient
    : public std::enable_shared_from_this<UmaClient>
    , public QWidget {
public:
    static bool initialized();
    static auto create_or_get() -> gsl::not_null<std::shared_ptr<UmaClient>>;

protected:
    UmaClient();

    void setup();

private:
};

} // namespace Experimental
