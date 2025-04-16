#include <iostream>
#include <console.hpp>

auto main(int, char**) -> int
{

    console::make();
    console::writeline("hello test main!");

    return { NULL };
}
