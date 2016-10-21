cpplatex
========

[![Build Status](https://travis-ci.org/zmarcantel/cpplatex.svg?branch=master)](https://travis-ci.org/zmarcantel/cpplatex)

[Documentation](https://zmarcantel.github.io/cpplatex)

Header-only `LaTeX` document builder in C++14.

An important feature is the ability to do (templated) math operations and get out not only the arithmetic
result, but a fully `LaTeX`-formatted representation.

This allows the user to do complex/scientific math using external libraries while being able to
programmatically generate reports, documentation, etc. from the calculations themselves with almost no effort.


quick example
=============

For more in-depth examples, see the tests.

```cpp
#include "cpplatex.hpp"

...

#define NUM(x)      latex::math::make_num(x)
#define EXP(x, y)   latex::math::make_exp(x, y)
#define ROOT(x, y)  latex::math::make_root(x, y)

static latex::math::Number<double> PI(3.14);

void find_area_and_volume(double height, double radius) {
    auto area = PI * EXP(radius, 2);
    auto volume = PI * pow(radius, 2) * height;

    std::cout << area << std::endl;                 // output the LaTeX formatted string
    std::cout << area.solve() << std::endl;         // output the area of this base

    std::cout << volume << std::endl;               // output the LaTeX formatted string
    std::cout << volume.solve() << std::endl;       // output the volume of this cylinder
}


void complex_equation() {
    // (sqrt(PI * 4) + 14.68 + log3(10))^4
    // -----------------------------------
    //          cube_root(9)
    auto eqn = 
        ((PI * 4).sqrt() + 14.68 + NUM(10).log(3)).pow(4)
        /
        ROOT(9, 3);

    std::cout << eqn << std::endl;                  // output the LaTeX formatted string
    std::cout << eqn.solve() << std::endl;          // output the reduced value of this complex equation
}

void make_doc() {
    // you can use the builder pattern...

    latex::doc::Document<latex::doc::doctypes::Report> doc("Some Title", "And A Subtitle");
    doc.with_toc()                                          // use a table of contents
       .use("some_import")                                  // use the "some_import" package
       .use("another_import")                               // use another package
       .with_leading_content("some content to insert");     // add some content before we get to the sections


    // ... the streaming pattern

    latex::doc::Section intro("Introduction");
    intro << "Blah blah, oh, and blah.";

    doc << intro;


    // ... or a mix of both! handy dandy.


    // print your document

    std::cout << doc;

    // or store it

    auto doc_str = doc.to_string();

    // or build it into a std::ostream

    std::stringstream ss;
    doc.build(ss);
}
```
