#include "../latex.hpp"

#include "gtest/gtest.h"
 
using namespace latex;

//------------------------------------------------
//
// document generation
//
//------------------------------------------------

TEST(Document, builder_pattern) {
    doc::Document<doc::doctypes::Report> doc("Some Title", "And A Subtitle");
    doc.with_toc()
       .use("some_import")
       .use("another_import")
       .with_leading_content("some content to insert");

    EXPECT_EQ("Some Title", doc.title);
    EXPECT_EQ("And A Subtitle", doc.subtitle);
    EXPECT_EQ(true, doc.use_toc);

    std::vector<std::string> expect_imports{"some_import", "another_import"};
    EXPECT_EQ(expect_imports, doc.imports);

    std::vector<std::string> expect_leading{"some content to insert"};
    EXPECT_EQ(expect_leading, doc.leading_contents);

    EXPECT_EQ(0, doc.sections.size());
}

TEST(Document, empty_document) {
    doc::Document<doc::doctypes::Article> doc("Title");

    std::string expect = ""
    "\\documentclass[12pt]{article}"
    "\\title{Title}"
    ""
    "\\begin{document}"
    "\\maketitle"
    ""
    "\\end{document}"
    ;

    // strip newlines for simplicity
    auto got = doc.to_string();
    std::string::size_type pos = 0; // Must initialize
    while ( ( pos = got.find ("\n",pos) ) != std::string::npos ) { got.erase(pos, 1); }

    EXPECT_EQ(expect, got);
}

TEST(Document, sections) {
    doc::Document<doc::doctypes::Report> doc("Some Title", "And A Subtitle");

    doc::Section section("A Beautiful Section");
    section << "This is a thing I would like to talk about.\n";


    doc::Subsection subsection("A Most Important Subsection");
    subsection << "Here is some clarification on that thing I was talking about.\n";

    section << subsection;
    doc << section;

    doc::Section new_page_section("This section is forced to a new page!", true);
    new_page_section << "Just some stuff\n";

    doc << new_page_section;


    std::string expect = ""
    "\\documentclass[12pt]{report}\n"
    "\n"
    "\\title{Some Title \\\\ And A Subtitle}\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\\begin{document}\n"
    "\n"
    "\\maketitle\n"
    "\n"
    "\\tableofcontents\n"
    "\n"
    "\\newpage\n"
    "\n"
    "\n"
    "\n"
    "\\section{A Beautiful Section}\n"
    "\n"
    "This is a thing I would like to talk about.\n"
    "\n"
    "\n"
    "\\subsection{A Most Important Subsection}\n"
    "\n"
    "Here is some clarification on that thing I was talking about.\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\\newpage\n"
    "\n"
    "\\section{This section is forced to a new page!}\n"
    "\n"
    "Just some stuff\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\\end{document}\n";

    EXPECT_EQ(expect, doc.to_string());
}


//
// lists
//

TEST(List, with_latex) {
    doc::List<> list;

    // as of writing, Equation does not have a ::to_string()
    auto eqn = math::make_eqn(
        math::Variable<Text<style::Bold, style::Italic>>("x"),
        (math::make_num(7) * 8) / 5
    );

    list
        << "an item"
        << eqn;

    auto expect = 
        "\\begin{itemize}\n"
        "\t\\item an item\n"
        "\t\\item \\begin{equation}\n"
        "\\textbf{\\textit{x}} = \\frac{7 * 8}{5}\n"
        "\\end{equation}\n"
        "\n"
        "\\end{itemize}\n";

    EXPECT_EQ(expect, list.to_string());
}

TEST(List, with_to_string) {
    doc::List<> list;

    list
        << "an item"
        << Text<style::Bold, style::Italic>("some stylized text");

    auto expect = 
        "\\begin{itemize}\n"
        "\t\\item an item\n"
        "\t\\item \\textbf{\\textit{some stylized text}}\n"
        "\\end{itemize}\n";

    EXPECT_EQ(expect, list.to_string());
}

TEST(List, unordered) {
    doc::List<> list;
    list
        << "an item"
        << "another item"
        << "third item"
        << "and another bullet point";

    auto expect =
        "\\begin{itemize}\n"
        "\t\\item an item\n"
        "\t\\item another item\n"
        "\t\\item third item\n"
        "\t\\item and another bullet point\n"
        "\\end{itemize}\n";

    EXPECT_EQ(expect, list.to_string());
}

TEST(List, unordered_nested) {
    doc::List<> list;
    doc::List<> sublist;
    doc::List<> third_tier;

    third_tier << "this takes a lot of explaining!";
    sublist 
        << "a note"
        << "another consideration"
        << "one more!"
            << third_tier;

    list
        << "an item"
        << "need more info"
            << sublist
        << "third item"
        << "and another bullet point";

    auto expect =
        "\\begin{itemize}\n"
        "\t\\item an item\n"
        "\t\\item need more info\n"
        "\t\\begin{itemize}\n"
        "\t\t\\item a note\n"
        "\t\t\\item another consideration\n"
        "\t\t\\item one more!\n"
        "\t\t\\begin{itemize}\n"
        "\t\t\t\\item this takes a lot of explaining!\n"
        "\t\t\\end{itemize}\n"
        "\t\\end{itemize}\n"
        "\t\\item third item\n"
        "\t\\item and another bullet point\n"
        "\\end{itemize}\n";

    EXPECT_EQ(expect, list.to_string());
}


// ordered

TEST(List, ordered) {
    doc::List<doc::listtypes::Ordered> list;
    list
        << "an item"
        << "another item"
        << "third item"
        << "and another bullet point";

    auto expect =
        "\\begin{enumerate}\n"
        "\t\\item an item\n"
        "\t\\item another item\n"
        "\t\\item third item\n"
        "\t\\item and another bullet point\n"
        "\\end{enumerate}\n";

    EXPECT_EQ(expect, list.to_string());
}

TEST(List, ordered_nested) {
    doc::List<doc::listtypes::Ordered> list;
    doc::List<doc::listtypes::Ordered> sublist;
    doc::List<doc::listtypes::Ordered> third_tier;

    third_tier << "this takes a lot of explaining!";
    sublist 
        << "a note"
        << "another consideration"
        << "one more!"
            << third_tier;

    list
        << "an item"
        << "need more info"
            << sublist
        << "third item"
        << "and another bullet point";

    auto expect =
        "\\begin{enumerate}\n"
        "\t\\item an item\n"
        "\t\\item need more info\n"
        "\t\\begin{enumerate}\n"
        "\t\t\\item a note\n"
        "\t\t\\item another consideration\n"
        "\t\t\\item one more!\n"
        "\t\t\\begin{enumerate}\n"
        "\t\t\t\\item this takes a lot of explaining!\n"
        "\t\t\\end{enumerate}\n"
        "\t\\end{enumerate}\n"
        "\t\\item third item\n"
        "\t\\item and another bullet point\n"
        "\\end{enumerate}\n";

    EXPECT_EQ(expect, list.to_string());
}

//
// sections
//

TEST(DocumentSection, newpage) {
    doc::Section sect("Title", true);
    auto expect =
        "\n\n"
        "\\newpage"
        "\n\n"
        "\\section{Title}\n"
        "\n"
    ;
    EXPECT_EQ(expect, sect.latex());
}


//------------------------------------------------
//
// math::solve() correctness
//
//------------------------------------------------


template <typename T>
class CustomType {
public:
    using ValType = T;

    T val;

    CustomType(const T& val) : val(val) {}

    template <typename U>
    friend CustomType operator+(CustomType cust, const U& other) {
        return CustomType(cust.val + other);
    }

    template <typename U>
    friend CustomType operator-(CustomType cust, const U& other) {
        return CustomType(cust.val - other);
    }

    template <typename U>
    friend CustomType operator/(CustomType cust, const U& other) {
        return CustomType(cust.val / other);
    }

    template <typename U>
    friend CustomType operator*(CustomType cust, const U& other) {
        return CustomType(cust.val * other);
    }
};


//
// addition
//

TEST(MathSolve, addition) {
    math::Addition<int> add(5, 7);
    EXPECT_EQ(12, add.solve());
}

TEST(MathSolve, addition_pod_plus_latex) {
    auto result = 5 + (7 + math::make_num(3));
    EXPECT_EQ(15, result.solve());
}

TEST(MathSolve, addition_custom_type) {
    auto result = math::make_add(CustomType<int>(5), 10);
    EXPECT_EQ(15, result.solve().val);
}

//
// subtraction
//

TEST(MathSolve, subtraction) {
    math::Subtraction<int> sub(18, 6);
    EXPECT_EQ(12, sub.solve());
}

TEST(MathSolve, subtraction_pod_minus_latex) {
    auto result = 5 - (math::make_num(7) - 4);
    EXPECT_EQ(2, result.solve());
}

// inherently tests negative after subtraction
TEST(MathSolve, subtraction_custom_type) {
    auto result = math::make_sub(CustomType<int>(5), 10);
    EXPECT_EQ(-5, result.solve().val);
}


//
// multiplication
//

TEST(MathSolve, multiplication) {
    math::Multiplication<int> mul(18, 6);
    EXPECT_EQ(108, mul.solve());
}

TEST(MathSolve, multiplication_pod_times_latex) {
    auto result = 5 * (1 + math::make_num(3));
    EXPECT_EQ(20, result.solve());
}


TEST(MathSolve, multiplication_custom_type) {
    auto result = math::make_mult(CustomType<int>(5), 10);
    EXPECT_EQ(50, result.solve().val);
}


//
// division
//

TEST(MathSolve, fraction_ints) {
    math::Fraction<int> as_ints(1, 2);
    EXPECT_EQ(0, as_ints.solve());
}

TEST(MathSolve, fraction_doubles) {
    math::Fraction<double> as_doubles(1, 2);
    EXPECT_EQ(0.5, as_doubles.solve());
}

TEST(MathSolve, fraction_int_over_double) {
    math::Fraction<int, double> int_over_double(1, 2);
    EXPECT_EQ(0.5, int_over_double.solve());
}

TEST(MathSolve, fraction_double_over_int) {
    math::Fraction<int, double> double_over_int(1, 2);
    EXPECT_EQ(0.5, double_over_int.solve());
}

TEST(MathSolve, fraction_pod_over_latex) {
    auto result = 30 / (math::make_num(2) * 3);
    EXPECT_EQ(5, result.solve());
}

TEST(MathSolve, fraction_custom_type) {
    math::Fraction<CustomType<float>, double> custom(8, 32);
    EXPECT_EQ(0.25, custom.solve().val);
}


//
// exponent
//

TEST(MathSolve, power) {
    math::Power<int> exp(2, 3);
    EXPECT_EQ(8, exp.solve());
}

//
// root
//

TEST(MathSolve, root) {
    math::Root<int> root(16, 2);
    EXPECT_EQ(4, root.solve());
}

TEST(MathSolve, root_non_square) {
    math::Root<int> root(16, 4);
    EXPECT_EQ(2, root.solve());
}

TEST(MathSolve, root_sqrt_fn) {
    auto root = math::make_num(64).sqrt();
    EXPECT_EQ(math::make_root(64, 2).solve(), root.solve());
    EXPECT_EQ(8, root.solve());
}


//
// log
//

TEST(MathSolve, log) {
    math::Log<int> log(16, 2);
    EXPECT_EQ(4, log.solve());
}


//
// natural log
//


TEST(MathSolve, ln) {
    math::NaturalLog<int> log(16);
    EXPECT_EQ(::log(16), log.solve());
}


//
// equation
//

// solve a basic equation
TEST(MathSolve, equation_basic) {
    math::Power<int, int> expo(4, 5);
    math::Log<int, int> logarithm(4, 10);

    math::Fraction<decltype(expo), decltype(logarithm)> fraction(expo, logarithm);
    math::Equation<std::string, decltype(fraction)> eq("Flow_{total}", fraction);

    double expect = pow(4, 5) / (log(4) / log(10));
    EXPECT_EQ(expect, eq.solve());
}



//------------------------------------------------
//
// math::latex() correctness
//
//------------------------------------------------

//
// addition
//

TEST(MathLatex, addition) {
    auto expect = "1 + 2";

    auto result = math::make_add(1, 2);
    EXPECT_EQ(expect, result.latex());
}

//
// subtraction
//

TEST(MathLatex, subtraction) {
    auto expect = "1 - 2";

    auto result = math::make_sub(1, 2);
    EXPECT_EQ(expect, result.latex());
}


//
// multiplication
//

TEST(MathLatex, multiplication) {
    auto expect = "1 * 2";

    auto result = math::make_mult(1, 2);
    EXPECT_EQ(expect, result.latex());
}

//
// division
//

TEST(MathLatex, fraction) {
    auto expect = "\\frac{1}{2}";

    auto result = math::make_fraction(1, 2);
    EXPECT_EQ(expect, result.latex());
}


//
// exponent
//

TEST(MathLatex, exponent) {
    auto expect = "{\\left(1\\right)}^{2}";

    auto result = math::make_pow(1, 2);
    EXPECT_EQ(expect, result.latex());
}


//
// root
//

TEST(MathLatex, sqrt) {
    auto expect = "\\sqrt{2}";

    auto result = math::make_root(2, 2);
    EXPECT_EQ(expect, result.latex());
    EXPECT_EQ(math::Number<int>(2).sqrt().latex(), result.latex());
}

TEST(MathLatex, root) {
    auto expect = "\\sqrt[3]{9}";

    auto result = math::make_root(9, 3);
    EXPECT_EQ(expect, result.latex());
}


//
// log
//

TEST(MathLatex, log) {
    auto expect = "\\log_{2}{\\left(9\\right)}";

    auto result = math::make_log(9, 2);
    EXPECT_EQ(expect, result.latex());
}

//
// equation
//

TEST(MathLatex, equation) {
    auto expect = 
        "\\begin{equation}\n"
        "\\textit{\\textbf{R}}_{flow} = \\frac{2 + 5 * {\\left(\\log_{2}{\\left(6.45\\right)} + 2\\right)}^{3}}{\\sqrt{4 * 3}}\n"
        "\\end{equation}\n";

    auto result =
        (math::make_num(2) + 5 * (math::make_num(6.45).log<>(2) + 2).pow(3))
        /
        (math::make_num(4) * 3).sqrt();

    auto eqn = math::make_eqn(
        math::SubscriptedVariable<Text<style::Italic, style::Bold>, std::string>("R", "flow"),
        result
    );

    EXPECT_EQ(expect, eqn.latex());
}

TEST(MathLatex, aligned_equation) {
    auto expect =
        "\\begin{equation}\n"
        "\\begin{split}\n"
        "\\textit{\\textbf{R}}_{flow} & = \\frac{2 + 5 * {\\left(\\log_{2}{\\left(6.45\\right)} + 2\\right)}^{3}}{\\sqrt{4 * 3}}\\\\\n"
        " & = 149.412\n"
        "\\end{split}\n"
        "\\end{equation}\n";

    auto result =
        (math::make_num(2) + 5 * (math::make_num(6.45).log<>(2) + 2).pow(3))
        /
        (math::make_num(4) * 3).sqrt();

    auto eqn = math::make_aligned_eqn(
        math::SubscriptedVariable<Text<style::Italic, style::Bold>, std::string>("R", "flow"),
        result,
        result.solve()
    );

    EXPECT_EQ(expect, eqn.latex());
}



//------------------------------------------------
//
// math operator correctness
//
//------------------------------------------------

TEST(MathArtihmetic, order_of_operations) {
    math::Fraction<> frac(1, 2);
    auto found = (frac * 2).pow(3) + 5 * 4;
    auto expect = pow(1, 3) + 5 * 4;
    EXPECT_EQ(expect, found.solve());
    EXPECT_EQ(21, expect);
}

auto plus_quadratic(int pre_a, int pre_b, int pre_c) {
    auto a = math::make_num(pre_a);
    auto b = math::make_num(pre_b);
    auto c = math::make_num(pre_c);

    return (-b + (b.pow(2) - 4 * a * c).sqrt()) / (2 * a);
}

TEST(MathArtihmetic, quadratic_eqn) {
    int a = 2;
    int b = 5;
    int c = -3;

    auto quad = plus_quadratic(a, b, c);
    auto expect = ((-b) + pow(pow(b, 2) - 4 * a * c, 0.5)) / (2 * a);

    EXPECT_EQ(expect, quad.solve());
}

//------------------------------------------------
// test runner main -- leave at EOF
//------------------------------------------------

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
