#include <iostream>
#include <console.hpp>
#include <GDIWindow.hpp>



auto main(int, char**) -> int
{
    console::make();
    console::writeline("hello gdimain");

    return { NULL };
}
