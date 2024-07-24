#include <MaaPP/MaaPP.hpp>

using namespace maa;

coro::Promise<int> async_main() {
    //! TODO: impl everything here
    co_return 0;
}

int main(int argc, char *argv[]) {
    coro::EventLoop ev;
    ev.stop_after(async_main());
    return ev.exec();
}
