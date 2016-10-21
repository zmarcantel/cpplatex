#ifndef CPPLATEX_LATEX_HPP
#define CPPLATEX_LATEX_HPP

#include <cmath>
#include <stack>
#include <string>
#include <vector>
#include <sstream>

namespace latex {

constexpr const static char* oparen = "\\left(";
constexpr const static char* cparen = "\\right)";
constexpr const static char* obracket = "\\left[";
constexpr const static char* cbracket = "\\right]";
constexpr const static char* obrace = "\\left{";
constexpr const static char* cbrace = "\\right}";


namespace style {
    //
    // font style
    //

    class None {
    public:
        constexpr const static char* open = "";
        constexpr const static char* close = "";
    };

    class Normal {
    public:
        constexpr const static char* open = "\\normal{";
        constexpr const static char* close = "}";
    };

    class Italic {
    public:
        constexpr const static char* open = "\\textit{";
        constexpr const static char* close = "}";
    };

    class Bold {
    public:
        constexpr const static char* open = "\\textbf{";
        constexpr const static char* close = "}";
    };

    class Underline {
    public:
        constexpr const static char* open = "\\underline{";
        constexpr const static char* close = "}";
    };

    //
    // text sizing
    //

    class Tiny {
    public:
        constexpr const static char* open = "\\tiny{";
        constexpr const static char* close = "}";
    };

    class Small {
    public:
        constexpr const static char* open = "\\small{";
        constexpr const static char* close = "}";
    };

    class Large {
    public:
        constexpr const static char* open = "\\large{";
        constexpr const static char* close = "}";
    };

    class Larger {
    public:
        constexpr const static char* open = "\\Large{";
        constexpr const static char* close = "}";
    };

    class Largest {
    public:
        constexpr const static char* open = "\\LARGE{";
        constexpr const static char* close = "}";
    };

    class Huge {
    public:
        constexpr const static char* open = "\\huge{";
        constexpr const static char* close = "}";
    };

    class Huger {
    public:
        constexpr const static char* open = "\\Huge{";
        constexpr const static char* close = "}";
    };
}



/** Simple text wrapper that allows styling and embedding text in non-textual contexts.
 *
 * Multiple style templates can be passed in to alter the text.
 */
template <typename Style=style::None, typename... Rest>
class Text {
public:
    std::string raw;

    Text(const char* t) : raw(std::string(t)) {}
    Text(const std::string& t) : raw(t) {}

    std::string str() const {
        std::stringstream ss;
        build(ss);
        return ss.str();
    }
    friend std::ostream& operator<<(std::ostream& os, const Text& t) {
        t.build(os);
        return os;
    }

    void build(std::ostream& ss) const {
        print_open(ss, Style::open, Rest::open...);

        ss << raw;

        print_close(ss, Style::close, Rest::close...);
    }

protected:
    template <typename First, typename... T>
    void print_open(std::ostream& ss, First f) const { ss << f; }

    template <typename First, typename... T>
    void print_open(std::ostream& ss, First f, T... rest) const {
        ss << f;
        print_open(ss, rest...);
    }

    template <typename First, typename... T>
    void print_close(std::ostream& ss, First f) const { ss << f; }

    template <typename First, typename... T>
    void print_close(std::ostream& ss, First f, T... rest) const {
        print_close(ss, rest...);
        ss << f;
    }
};

using ItalicText = Text<style::Italic>;
using BoldText = Text<style::Bold>;
using UnderlineText = Text<style::Underline>;


//------------------------------------------------
//
// document generation
//
//------------------------------------------------

namespace doc {
    namespace doctypes {
        /** Use the 'article' doctype for document generation.
         */
        class Article {
            public:
            constexpr const static char* header = "article";
            constexpr const static bool can_toc = false;
            constexpr const static bool can_subtitle = false;
        };

        /** Use the 'report' doctype for document generation.
         */
        class Report {
            public:
            constexpr const static char* header = "report";
            constexpr const static bool can_toc = true;
            constexpr const static bool can_subtitle = true;
        };

        /** Use the 'book' doctype for document generation.
         */
        class Book {
            public:
            constexpr const static char* header = "book";
            constexpr const static bool can_toc = true;
            constexpr const static bool can_subtitle = true;
        };
    }

    // TODO: "infinitely" nested subsections
    /** Defines a subsection (\subsection{}) within a section.
     *
     * Supports streaming in (sub << "my string") and streaming out (section << sub).
     *
     * Alternatively, use `Section::add_subsection(...)`.
     */
    class Subsection {
        protected:
            std::string title;
            std::vector<std::string> content;

        public:
            Subsection(const std::string& title) : title(title) {}

            Subsection& operator<<(const char* val) {content.push_back(std::string(val)); return *this;}
            Subsection& operator<<(const std::string& val) {content.push_back(val); return *this;}

            friend std::ostream& operator<<(std::ostream& os, const Subsection& sub) {
                os << "\\subsection{" << sub.title << "}\n\n";

                for (auto& c : sub.content) {
                    os << c << "\n\n";
                }

                return os;
            }
    };


    /** Defines a section (\section{}) in the document.
     *
     * Output is as follows:
     *   - title
     *   - leading content
     *   - subsections[]
     *
     * Subsections can be streamed into the section (section << sub) or added using `::add_subsection(...)`.
     */
    class Section {
        protected:
            std::string title;
            std::vector<Subsection> subs;
            std::vector<std::string> leading_content;

        public:
            Section(const std::string& title) : title(title) {}

            Section& operator<<(const char* val) {leading_content.push_back(std::string(val)); return *this;}
            Section& operator<<(const std::string& val) {leading_content.push_back(val); return *this;}
            Section& operator<<(const Subsection& val) {subs.push_back(val); return *this;}

            friend std::ostream& operator<<(std::ostream& os, const Section& sect) {
                os << "\\section{" << sect.title << "}\n\n";

                for (auto& c : sect.leading_content) {
                    os << c << "\n\n";
                }

                for (auto& s : sect.subs) {
                    os << s << "\n\n";
                }

                return os;
            }

            void add_subsection(const Subsection& sub) { *this << sub; }
    };


    /** Defines a document which is the root for generating a valid LaTeX document.
     *
     * Documents must be given a title, and optionally a subtitle.
     *
     * If the document type supports a table of contents, one is generated after the title page.
     * Table of contents can be toggled via the `::use_toc` variable or `::with_toc()` function.
     *
     * The `::use()` function allows importing LaTeX packages.
     *
     * Leading content is output first, followed by all sections (and their subsections).
     */
    template<typename DocType=doctypes::Article>
    class Document {
        public:
            std::string title;
            std::string subtitle;
            bool use_toc;

            std::vector<std::string> imports;
            std::vector<std::string> leading_contents;
            std::vector<Section> sections;

            //
            // functions
            //

            Document(const std::string& title) : Document(title, "") {}
            Document(const std::string& title, const std::string& subtitle)
                : title(title), subtitle(subtitle), use_toc(DocType::can_toc)
            {}

            /** Import a package into the document */
            Document& use(const std::string& import) { imports.push_back(import); return *this; }

            /** Builder pattern method for adding leading content */
            Document& with_leading_content(const std::string& content) { write_leading(content); return *this; }


            /** Enable the table of contents if allowed by the document type */
            Document& with_toc() { use_toc = DocType::can_toc; return *this; }

            /** Write leading text (what comes before the sections in the document) into the document. */
            void write_leading(const std::string& content) { leading_contents.push_back(content); }
            void insert(const Section& sect) { sections.push_back(sect); }

            Document& operator<<(const char* val) {write_leading(std::string(val)); return *this;}
            Document& operator<<(const std::string& val) {write_leading(val); return *this;}

            Document& operator<<(const Section& val) {
                sections.push_back(val);
                return *this;
            }


            friend std::ostream& operator<<(std::ostream& os, const Document& doc) { doc.build(os); return os; }

            /** Get the entire, LaTeX-formatted, document in a std::string. */
            std::string to_string() const {
                std::stringstream ss;
                build(ss);
                return ss.str();
            }

            /** Helper for outputting the document to a generic std::ostream */
            void build(std::ostream& ss) const {
                ss << "\\documentclass{" << DocType::header << "}\n"
                   << "\n";

                if (DocType::can_subtitle and subtitle.size()) {
                    ss << "\\title{" << title << " \\\\ " << subtitle << "}\n";
                } else {
                    ss << "\\title{" << title << "}\n";
                }

                ss << "\n\n";
                for (auto& i : imports) {
                    ss << "\\usepackage{" << i << "}\n";
                }
                ss << "\n\n";

                ss << "\\begin{document}\n"
                   << "\n"
                   << "\\maketitle\n"
                   << "\n"
                   << (use_toc ? "\\tableofcontents\n\n\\newpage\n\n" : "")
                ;

                ss << "\n\n";
                for (auto& c : leading_contents) {
                    ss << c << "\n\n";
                }

                for (auto& s : sections) {
                    ss << s << "\n\n";
                }

                ss << "\\end{document}\n";
            }
    };

} // doc namespace


//------------------------------------------------
//
// math/equation generation
//
//------------------------------------------------

namespace math {
    //
    // ::solve() enable_if "interface"
    //
    template <typename T> auto solve_impl(int) -> decltype(std::declval<T>().solve(), std::true_type{});
    template <typename T> auto solve_impl(...) -> std::false_type;
    template <typename T> using can_solve = decltype(solve_impl<T>(0));

    template <typename T, typename = typename std::enable_if<can_solve<T>::value>::type>
    auto reduce(T& val) { return val.solve(); }

    template <typename T>
    typename std::enable_if<
        not can_solve<T>::value || std::is_arithmetic<T>::value
        , T
    >::type
    reduce(T& val) { return val; }


    // unfortunate fwdecls
    template <typename LHS, typename RHS> class Addition;
    template <typename LHS, typename RHS> class Subtraction;
    template <typename LHS, typename RHS> class Fraction;
    template <typename LHS, typename RHS> class Multiplication;
    template <typename LHS, typename RHS> class Exponent;
    template <typename Val, typename Power> class Root;
    template <typename Val, typename Base> class Log;
    template <typename Val> class NaturalLog;


    //
    // artihmetic base class
    //

    /** Helper class for adding arithmetic overloads to other math operations. */
    template <typename Self>
    class Arithmetic {
    public:
        template<typename T> using AdditionType = Addition<Self, T>;
        template<typename T> using SubtractionType = Subtraction<Self, T>;
        template<typename T> using MultiplicationType = Multiplication<Self, T>;
        template<typename T> using FractionType = Fraction<Self, T>;
        template<typename T> using ExponentType = Exponent<Self, T>;
        template<typename B> using LogType = Log<Self, B>;
        using NaturalLogType = NaturalLog<Self>;

        template <typename T>
        friend AdditionType<T> operator+(Self lhs, const T& rhs) { return AdditionType<T>(lhs, rhs); }

        template <typename T>
        friend SubtractionType<T> operator-(Self lhs, const T& rhs) { return SubtractionType<T>(lhs, rhs); }

        template <typename T>
        friend MultiplicationType<T> operator*(Self lhs, const T& rhs) { return MultiplicationType<T>(lhs, rhs); }

        template <typename T>
        friend FractionType<T> operator/(Self lhs, const T& rhs) { return FractionType<T>(lhs, rhs); }
    };



    /** Helper class for "entering the LaTeX context" using a constant numeric.
     *
     * Most commonly used to then access the overloaded operators.
     */
    template <typename T>
    class Number : public Arithmetic<Number<T>> {
    protected:
        T val;
    
    public:
        using Self = Number<T>;

        Number(T val) : val(val) {}

        auto solve() { return reduce(val); }

        std::string latex() const {
            std::stringstream ss;
            ss << val;
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Number& num) { os << num.latex(); return os; }

        Self operator-() { return Number<T>(0 - val); }

        template <typename Pow> Exponent<Self, Pow> pow(const Pow& power) { return Exponent<Self, Pow>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename T>
    auto make_num(const T& val) { return Number<T>(val); }


    /** Helper class for keeping LaTeX equations sane by wrapping sections in parentheses.
     */
    template <typename T>
    class Paren {
    public:
        T enclosed;

        Paren(T enc) : enclosed(enc) {}

        auto solve() { return reduce(enclosed); }

        std::string latex() const {
            std::stringstream ss;
            ss << ::latex::oparen << enclosed << ::latex::cparen;
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Paren& par) { os << par.latex(); return os; }
    };

    template <typename T>
    auto make_paren(const T& val) { return Paren<T>(val); }


    //
    // math operations
    //


    template <typename Numerator=double, typename Denominator=Numerator>
    class Fraction : public Arithmetic<Fraction<Numerator, Denominator>> {
    protected:
        Numerator num;
        Denominator den;
    
    public:
        using NumeratorType = Numerator;
        using DenominatorType = Denominator;
        using Self = Fraction<Numerator, Denominator>;

        Fraction(Numerator num, Denominator den) : num(num), den(den) {}

        auto solve() {
            return reduce(num) / reduce(den);
        }

        std::string latex() const {
            std::stringstream ss;
            ss << "\\frac{" << num << "}{" << den << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Fraction& frac) { os << frac.latex(); return os; }

        template <typename T> Exponent<Self, T> pow(const T& power) { return Exponent<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename LHS, typename RHS>
    auto make_fraction(const LHS& lhs, const RHS& rhs) { return Fraction<LHS, RHS>(lhs, rhs); }


    template <typename LHS, typename RHS=LHS>
    class Multiplication : public Arithmetic<Multiplication<LHS, RHS>> {
    protected:
        LHS lhs;
        RHS rhs;
    
    public:
        using LHSType = LHS;
        using RHSType = RHS;
        using Self = Multiplication<LHS, RHS>;

        Multiplication(LHS lhs, RHS rhs) : lhs(lhs), rhs(rhs) {}
    
        auto solve() { return reduce(lhs) * reduce(rhs); }
        std::string latex() const {
            std::stringstream ss;
            ss << lhs << " * " << rhs;
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Multiplication& mult) { os << mult.latex(); return os; }

        template <typename T> Exponent<Self, T> pow(const T& power) { return Exponent<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename LHS, typename RHS>
    auto make_mult(const LHS& lhs, const RHS& rhs) { return Multiplication<LHS, RHS>(lhs, rhs); }

    template <typename LHS, typename RHS=LHS>
    class Addition : public Arithmetic<Addition<LHS, RHS>> {
    protected:
        LHS lhs;
        RHS rhs;
    
    public:
        using LHSType = LHS;
        using RHSType = RHS;
        using Self = Addition<LHS, RHS>;

        Addition(LHS lhs, RHS rhs) : lhs(lhs), rhs(rhs) {}
    
        auto solve() { return reduce(lhs) + reduce(rhs); }
        std::string latex() const {
            std::stringstream ss;
            ss << lhs << " + " << rhs;
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Addition& add) { os << add.latex(); return os; }

        template <typename T> Exponent<Self, T> pow(const T& power) { return Exponent<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename LHS, typename RHS>
    auto make_add(const LHS& lhs, const RHS& rhs) { return Addition<LHS, RHS>(lhs, rhs); }

    template <typename LHS, typename RHS=LHS>
    class Subtraction : public Arithmetic<Subtraction<LHS, RHS>> {
    protected:
        LHS lhs;
        RHS rhs;
    
    public:
        using LHSType = LHS;
        using RHSType = RHS;
        using Self = Subtraction<LHS, RHS>;

        Subtraction(LHS lhs, RHS rhs) : lhs(lhs), rhs(rhs) {}
    
        auto solve() { return reduce(lhs) - reduce(rhs); }
        std::string latex() const {
            std::stringstream ss;
            ss << lhs << " - " << rhs;
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Subtraction& sub) { os << sub.latex(); return os; }

        template <typename T> Exponent<Self, T> pow(const T& power) { return Exponent<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename LHS, typename RHS>
    auto make_sub(const LHS& lhs, const RHS& rhs) { return Subtraction<LHS, RHS>(lhs, rhs); }

    template <typename LHS, typename RHS=LHS>
    class Exponent : public Arithmetic<Exponent<LHS, RHS>> {
    protected:
        LHS lhs;
        RHS rhs;
    
    public:
        using LHSType = LHS;
        using RHSType = RHS;
        using Self = Exponent<LHS, RHS>;

        Exponent(LHS lhs, RHS rhs) : lhs(lhs), rhs(rhs) {}
    
        auto solve() { return ::pow(reduce(lhs), reduce(rhs)); }
        std::string latex() const {
            std::stringstream ss;
            ss << "{" << ::latex::oparen << lhs << ::latex::cparen << "}^{" << rhs << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Exponent& expo) { os << expo.latex(); return os; }

        template <typename T> Exponent<Self, T> pow(const T& power) { return Exponent<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename LHS, typename RHS>
    auto make_exp(const LHS& lhs, const RHS& rhs) { return Exponent<LHS, RHS>(lhs, rhs); }

    template <typename Val, typename Power=Val>
    class Root : public Arithmetic<Root<Val, Power>> {
    protected:
        Val val;
        Power base;
    
    public:
        using ValType = Val;
        using BaseType = Power;
        using Self = Root<Val, Power>;

        Root(Val val, Power base) : val(val), base(base) {}
    
        auto solve() { return ::pow(reduce(val), ((double)1.0)/reduce(base)); }
        std::string latex() const {
            std::stringstream ss;
            ss << '\\' << "sqrt";
            if (reduce(base) != 2) { ss << "[" << base << "]"; }
            ss  << "{" << val << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Root& r) { os << r.latex(); return os; }

        template <typename T> Exponent<Self, T> pow(const T& power) { return Exponent<Self, T>(*this, power); }

        template <typename InnerPower> Log<Self, InnerPower> log() { return Log<Self, InnerPower>(*this); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename Val, typename Base=Val>
    auto make_root(const Val& val, const Base& base) { return Root<Val, Base>(val, base); }

    template <typename Val, typename Base=Val>
    class Log : public Arithmetic<Log<Val, Base>> {
    protected:
        Val val;
        Base base;
    
    public:
        using ValType = Val;
        using BaseType = Base;
        using Self = Root<Val, Base>;

        Log(Val val, Base base) : val(val), base(base) {}
    
        auto solve() { return ::log(reduce(val)) / ::log(base); }
        std::string latex() const {
            std::stringstream ss;
            ss << '\\' << "log_{" << base << "}{" << oparen <<  val << cparen << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Log& loga) { os << loga.latex(); return os;}

        template <typename T> Exponent<Self, T> pow(const T& power) { return Exponent<Self, T>(*this, power); }

        template <typename InnerBase> Log<Self, Base> log() { return Log<Self, InnerBase>(*this); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename Val, typename Base>
    auto make_log(const Val& val, const Base& base) { return Log<Val, Base>(val, base); }

    template <typename Val>
    class NaturalLog : public Arithmetic<NaturalLog<Val>> {
    protected:
        Val val;
    
    public:
        using ValType = Val;
        using Self = NaturalLog<Val>;

        NaturalLog(Val val) : val(val) {}
    
        auto solve() { return ::log(reduce(val)); }
        std::string latex() const {
            std::stringstream ss;
            ss << "\\ln{" << val << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const NaturalLog& loga) { os << loga.latex(); return os;}

        template <typename T> Exponent<Self, T> pow(const T& power) { return Exponent<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename Val>
    auto make_ln(const Val& val) { return NaturalLog<Val>(val); }


    //
    // composed equations
    //


    template<typename LHS, typename RHS>
    class Equation {
    protected:
        LHS lhs;
        RHS rhs;
        std::string label;

    public:
        using LHSType = LHS;
        using RHSType = RHS;

        Equation(LHS lhs, RHS rhs) : lhs(lhs), rhs(rhs) {}

        Equation(const std::string& label, LHS lhs, RHS rhs) : lhs(lhs), rhs(rhs), label(label) {}

        /** This **does not** solve for variables or balance and solve equations.
         *
         * It simply reduces the right hand side.
         */
        auto solve() { return rhs.solve(); }
        std::string latex() const {
            std::stringstream ss;
            ss << "\\begin{equation}";
            if (label.size()) {
                ss << "\\label{eq:" << label << "}\n";
            } else {
                ss << "\n";
            }

            ss << lhs << " = " << rhs << "\n";

            ss << "\\end{equation}\n";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Equation& eq) { os << eq.latex(); return os; }
    };

    template <typename LHS, typename RHS>
    auto make_eqn(const LHS& lhs, const RHS& rhs) { return Equation<LHS, RHS>(lhs, rhs); }


    // TODO: aligned equations
    // TODO: reference other equation(s)' label


    //
    // text
    //

    template <typename T=std::string>
    class Variable {
    public:
        T name;

        Variable(const T& name) : name(name) {}

        void solve() {
            std::stringstream ss;
            ss << "attempted to solve an equation containing variable '" << name << "'\n";
            throw std::runtime_error(ss.str());
        }

        std::string latex() const {
            std::stringstream ss;
            ss << name;
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Variable& var) {
            os << var.latex();
            return os;
        }
    };

    template <typename Upper, typename Lower=Upper>
    class SubscriptedVariable {
    public:
        Upper upper;
        Lower lower;

        SubscriptedVariable(const Upper& up, const Lower& low) : upper(up), lower(low) {}

        void solve() {
            std::stringstream ss;
            ss << "attempted to solve an equation containing variable '" << upper << "_" << lower << "'\n";
            throw std::runtime_error(ss.str());
        }

        std::string latex() const {
            std::stringstream ss;
            ss << upper << "_{" << lower << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const SubscriptedVariable& var) {
            os << var.latex();
            return os;
        }
    };
} // math namespace



} // latex namespace
#endif
