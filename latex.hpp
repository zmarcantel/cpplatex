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

    std::string to_string() const {
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

using TinyText = Text<style::Tiny>;
using SmallText = Text<style::Small>;
using LargeText = Text<style::Large>;
using LargerText = Text<style::Larger>;
using LargestText = Text<style::Largest>;
using HugeText = Text<style::Huge>;
using HugerText = Text<style::Huger>;


//------------------------------------------------
//
// document generation
//
//------------------------------------------------

namespace doc {
    //
    // ::to_string() enable_if "interface"
    //
    template <typename T> auto to_string_impl(int) -> decltype(std::declval<T>().to_string(), std::true_type{});
    template <typename T> auto to_string_impl(...) -> std::false_type;
    template <typename T> using can_stringify = decltype(to_string_impl<T>(0));

    //
    // ::latex() enable_if "interface"
    //
    template <typename T> auto latex_impl(int) -> decltype(std::declval<T>().latex(), std::true_type{});
    template <typename T> auto latex_impl(...) -> std::false_type;
    template <typename T> using can_latex = decltype(latex_impl<T>(0));



    namespace listtypes {
        /** Specifies the list should be represented in an ordered (numbered rather than bulleted) form. */
        class Ordered {
        public:
            constexpr const static char* open = "\\begin{enumerate}";
            constexpr const static char* close = "\\end{enumerate}";
        };

        /** Specifies the list should be represented in a bulleted format */
        class Unordered {
        public:
            constexpr const static char* open = "\\begin{itemize}";
            constexpr const static char* close = "\\end{itemize}";
        };
    }

    /** Create a LaTeX formatted list.
     *
     * Nested lists must be of the same style (i.e. cannot mix Ordered and Unordered).
     */
    template <typename Style=listtypes::Unordered>
    class List {
    protected:
        enum class EntryType : std::uint8_t {
            String,
            Sublist,
        };
        std::vector<EntryType> ordering;

    public:
        std::vector<std::string> items;
        std::vector<List<Style>> sublists;

        //
        // input
        //

        List<Style>& operator<<(const char* val) { add(std::string(val)); return *this; }
        List<Style>& operator<<(const std::string& val) { add(val); return *this; }

        List<Style>& operator<<(List<Style>& val) { add_sublist(val); return *this; }
        List<Style>& operator<<(const List<Style>& val) { add_sublist(val); return *this; }

        template <
            typename T,
            typename = typename std::enable_if<
                can_stringify<T>::value
                and
                (not std::is_same<T, List<listtypes::Ordered>>::value and not std::is_same<T, List<listtypes::Unordered>>::value)
            >::type
        >
        List<Style>& operator<<(const T& val) { add(val.to_string()); return *this; }

        template <typename T>
        typename std::enable_if<
            can_latex<T>::value
            , List<Style>&
        >::type
        operator<<(const T& val) { add(val.latex()); return *this; }

        void add(const std::string& str) {
            items.push_back(str);
            ordering.push_back(EntryType::String);
        }

        void add_sublist(const List<Style>& list) {
            sublists.push_back(list);
            ordering.push_back(EntryType::Sublist);
        }

        //
        // output
        //

        std::string to_string() const {
            std::stringstream ss;
            build(ss);
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, List& list) { list.build(os); return os; }

        void build(std::ostream& os, std::size_t depth = 1) const {
            auto itemit = items.begin();
            auto subit = sublists.begin();

            std::stringstream prefix;
            for (auto i = 1u; i < depth; ++i) { prefix << "\t"; } // start at 1 to make it one less than the constituent items

            os << prefix.str() << Style::open << std::endl;
            for (auto& type : ordering) {
                switch (type) {
                case EntryType::String:
                    if (itemit != items.end()) {
                        os << prefix.str() << "\t" /* one more */ << "\\item " << *itemit << std::endl;
                        itemit++;
                    } else {
                        throw std::runtime_error("attempted to output a list item, but no items remaining.");
                    }
                    break;

                case EntryType::Sublist:
                    if (subit != sublists.end()) {
                        (*subit).build(os, depth+1);
                        subit++;
                    } else {
                        throw std::runtime_error("attempted to output a sublist item, but no sublists remaining.");
                    }
                    break;
                }
            }
            os << prefix.str() << Style::close << std::endl;
        }
    };

    using OrderedList = List<listtypes::Ordered>;
    using UnorderedList = List<listtypes::Unordered>;


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

            template <typename T, typename std::enable_if<can_stringify<T>::value, bool>::type = 0>
            Subsection& operator<<(const T& val) { content.push_back(val.to_string()); return *this; }

            template <typename T, typename std::enable_if<can_latex<T>::value, char>::type = 0>
            Subsection& operator<<(const T& val) { content.push_back(val.latex()); return *this; }

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
            bool new_page;
            std::string title;
            std::vector<Subsection> subs;
            std::vector<std::string> leading_content;

        public:
            Section(const std::string& title, bool new_page = false) : new_page(new_page), title(title) {}

            Section& operator<<(const char* val) {leading_content.push_back(std::string(val)); return *this;}
            Section& operator<<(const std::string& val) {leading_content.push_back(val); return *this;}
            Section& operator<<(const Subsection& val) {subs.push_back(val); return *this;}

            template <typename T, typename = typename std::enable_if<can_stringify<T>::value>::type>
            Section& operator<<(const T& val) { leading_content.push_back(val.to_string()); return *this; }

            void build(std::ostream& os) const {
                if (new_page) {
                    os << "\n\n\\newpage\n\n";
                }

                os << "\\section{" << title << "}\n\n";

                for (auto& c : leading_content) {
                    os << c << "\n\n";
                }

                for (auto& s : subs) {
                    os << s << "\n\n";
                }
            }


            std::string latex() const {
                std::stringstream ss;
                build(ss);
                return ss.str();
            }

            friend std::ostream& operator<<(std::ostream& os, const Section& sect) {
                sect.build(os);
                return os;
            }

            void add_subsection(const Subsection& sub) { *this << sub; }
    };


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


    // TODO: part ordering scheme like in List
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
    template<typename DocType=doctypes::Article, std::uint8_t font_size = 12>
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
                ss << "\\documentclass[" << (std::size_t)font_size << "pt]{" << DocType::header << "}\n"
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

    using Article = Document<doctypes::Article>;
    using Report = Document<doctypes::Report>;
    using Book = Document<doctypes::Book>;

} // doc namespace


//------------------------------------------------
//
// math/equation generation
//
//------------------------------------------------

namespace math {
    namespace style {
        class None {
        public:
            constexpr const static char* open = "";
            constexpr const static char* close = "";
        };

        class Normal {
        public:
            constexpr const static char* open = "\\mathnormal{";
            constexpr const static char* close = "}";
        };

        class Italic {
        public:
            constexpr const static char* open = "\\mathit{";
            constexpr const static char* close = "}";
        };

        class Bold {
        public:
            constexpr const static char* open = "\\boldsymbol{";
            constexpr const static char* close = "}";
        };
    }

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
    template <typename LHS, typename RHS> class Power;
    template <typename Val, typename Power> class Root;
    template <typename Val, typename Base> class Log;
    template <typename Val> class NaturalLog;



    /** Helper class for "entering the LaTeX context" using a constant numeric.
     *
     * Most commonly used to then access the overloaded operators.
     */
    template <typename T>
    class Number {
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

        template <typename Pow> Power<Self, Pow> pow(const Pow& power) { return Power<Self, Pow>(*this, power); }

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
    class Fraction {
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

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename LHS, typename RHS>
    auto make_fraction(const LHS& lhs, const RHS& rhs) { return Fraction<LHS, RHS>(lhs, rhs); }


    template <typename LHS, typename RHS=LHS>
    class Multiplication {
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

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename LHS, typename RHS>
    auto make_mult(const LHS& lhs, const RHS& rhs) { return Multiplication<LHS, RHS>(lhs, rhs); }

    template <typename LHS, typename RHS=LHS>
    class Addition {
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

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename LHS, typename RHS>
    auto make_add(const LHS& lhs, const RHS& rhs) { return Addition<LHS, RHS>(lhs, rhs); }

    template <typename LHS, typename RHS=LHS>
    class Subtraction {
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

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename LHS, typename RHS>
    auto make_sub(const LHS& lhs, const RHS& rhs) { return Subtraction<LHS, RHS>(lhs, rhs); }



    template <typename LHS, typename RHS=LHS>
    class Power {
    protected:
        LHS lhs;
        RHS rhs;

    public:
        using LHSType = LHS;
        using RHSType = RHS;
        using Self = Power<LHS, RHS>;

        Power(LHS lhs, RHS rhs) : lhs(lhs), rhs(rhs) {}

        auto solve() { return ::pow(reduce(lhs), reduce(rhs)); }

        std::string latex() const {
            std::stringstream ss;
            ss << "{" << ::latex::oparen << lhs << ::latex::cparen << "}^{" << rhs << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Power& expo) { os << expo.latex(); return os; }

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename LHS, typename RHS>
    auto make_pow(const LHS& lhs, const RHS& rhs) { return Power<LHS, RHS>(lhs, rhs); }



    template <typename RHS>
    class Exponent {
    protected:
        RHS rhs;

    public:
        using RHSType = RHS;
        using Self = Exponent<RHS>;

        Exponent(RHS rhs) : rhs(rhs) {}

        auto solve() { return ::exp(reduce(rhs)); }

        std::string latex() const {
            std::stringstream ss;
            // TODO: explicit \mathit{} ok to assume?
            ss << "\\mathit{e}^{" << ::latex::oparen << rhs << ::latex::cparen << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Exponent& expo) { os << expo.latex(); return os; }

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename RHS>
    auto make_exp(const RHS& rhs) { return Exponent<RHS>(rhs); }



    template <typename Val, typename Pow=Val>
    class Root {
    protected:
        Val val;
        Pow base;


    public:
        using ValType = Val;
        using PowType = Pow;
        using Self = Root<Val, Pow>;

        Root(Val val, Pow base) : val(val), base(base) {}

        auto solve() { return ::pow(reduce(val), ((double)1.0)/reduce(base)); }

        std::string latex() const {
            std::stringstream ss;
            ss << '\\' << "sqrt";
            if (reduce(base) != 2) { ss << "[" << base << "]"; }
            ss  << "{" << val << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Root& r) { os << r.latex(); return os; }

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

        template <typename InnerPower> Log<Self, InnerPower> log() { return Log<Self, InnerPower>(*this); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename Val, typename Base=Val>
    auto make_root(const Val& val, const Base& base) { return Root<Val, Base>(val, base); }



    template <typename Val, typename Base=Val>
    class Log {
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

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

        template <typename InnerBase> Log<Self, Base> log() { return Log<Self, InnerBase>(*this); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename Val, typename Base>
    auto make_log(const Val& val, const Base& base) { return Log<Val, Base>(val, base); }



    template <typename Val>
    class NaturalLog {
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

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

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

        constexpr const static char* open_context = "\\begin{equation}";
        constexpr const static char* close_context = "\\end{equation}";

    public:
        using LHSType = LHS;
        using RHSType = RHS;

        Equation(LHS lhs, RHS rhs) : lhs(lhs), rhs(rhs) {}

        Equation(const std::string& label, LHS lhs, RHS rhs) : lhs(lhs), rhs(rhs), label(label) {}

        void insert_label(std::ostream& os) const {
            if (label.size()) {
                os << "\\label{eq:" << label << "}\n";
            } else {
                os << "\n";
            }
        }

        /** This **does not** solve for variables or balance and solve equations.
         *
         * Simply reduces the right hand side yielding a numeric value.
         * If a non-ValuedVariable variable is included, an exception should be thrown.
         */
        auto solve() { return rhs.solve(); }

        /** Output the latex formatted version of the eqation.
         */
        std::string latex() const {
            std::stringstream ss;
            ss << open_context;

            insert_label(ss);
            ss << lhs << " = " << rhs << "\n";

            ss << close_context << "\n";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Equation& eq) { os << eq.latex(); return os; }
    };

    template <typename LHS, typename RHS>
    auto make_eqn(const LHS& lhs, const RHS& rhs) { return Equation<LHS, RHS>(lhs, rhs); }


    template <typename LHS, typename... Steps>
    class AlignedEquation {
    protected:
        std::string eqn;
        // TODO: label

        constexpr const static char* open_context = "\\begin{equation}";
        constexpr const static char* close_context = "\\end{equation}";
        constexpr const static char* split_eq = " & = ";

        template <typename First, typename... T>
        void print_steps(std::ostream& os, First& f, T... rest) {
            os << split_eq << f << "\\\\\n";
            print_steps(os, rest...);
        }

        template <typename First, typename... T>
        void print_steps(std::ostream& os, First& f) {
            os << split_eq << f << "\n";
        }

    public:
        AlignedEquation(const LHS& lhs, Steps... steps) {
            std::stringstream ss;
            ss << open_context << "\n";

            ss << "\\begin{split}\n";

            ss << lhs;
            print_steps(ss, steps...);

            ss << "\\end{split}\n";
            ss << close_context << "\n";

            eqn = ss.str();
        }

        std::string latex() const { return eqn; }

        friend std::ostream& operator<<(std::ostream& os, const AlignedEquation& eq) { os << eq.latex(); return os;}
    };

    template <typename LHS, typename... Steps>
    auto make_aligned_eqn(const LHS& lhs, const Steps... steps) { return AlignedEquation<LHS, Steps...>(lhs, steps...); }


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

    template <typename T, typename StrType=std::string>
    class ValuedVariable {
    protected:
        bool use_name;

    public:
        T val;
        StrType name;

        using Self = ValuedVariable<T, StrType>;

        ValuedVariable(const T& val, const StrType& name, bool use_name=true)
            : use_name(use_name), val(val), name(name)
        {}

        auto solve() { return reduce(val); }

        std::string latex() const {
            std::stringstream ss;
            if (use_name) {
                ss << name;
            } else {
                ss << val;
            }
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const ValuedVariable& var) {
            os << var.latex();
            return os;
        }

        template <typename U> Power<Self, U> pow(const U& power) { return Power<Self, U>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
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



    template <typename Val>
    class Sin {
    protected:
        Val val;

    public:
        using ValType = Val;
        using Self = Sin<Val>;

        Sin(Val val) : val(val) {}

        auto solve() { return ::sin(reduce(val)); }
        std::string latex() const {
            std::stringstream ss;
            ss << "\\sin{" << latex::oparen << val << latex::cparen << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Sin& s) { os << s.latex(); return os;}

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename Val>
    auto make_sin(const Val& val) { return Sin<Val>(val); }



    template <typename Val>
    class Cos {
    protected:
        Val val;

    public:
        using ValType = Val;
        using Self = Cos<Val>;

        Cos(Val val) : val(val) {}

        auto solve() { return ::sin(reduce(val)); }
        std::string latex() const {
            std::stringstream ss;
            ss << "\\cos{" << latex::oparen << val << latex::cparen << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Cos& s) { os << s.latex(); return os;}

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename Val>
    auto make_cos(const Val& val) { return Cos<Val>(val); }



    template <typename Val>
    class Tan {
    protected:
        Val val;

    public:
        using ValType = Val;
        using Self = Tan<Val>;

        Tan(Val val) : val(val) {}

        auto solve() { return ::sin(reduce(val)); }
        std::string latex() const {
            std::stringstream ss;
            ss << "\\tan{" << latex::oparen << val << latex::cparen << "}";
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream& os, const Tan& s) { os << s.latex(); return os;}

        template <typename T> Power<Self, T> pow(const T& power) { return Power<Self, T>(*this, power); }

        template <typename Base> Log<Self, Base> log(Base base) { return Log<Self, Base>(*this, base); }

        NaturalLog<Self> ln() { return NaturalLog<Self>(*this); }

        Root<Self, int> sqrt() { return Root<Self, int>(*this, 2); }
    };

    template <typename Val>
    auto make_tan(const Val& val) { return Tan<Val>(val); }



} // math namespace



} // latex namespace


//
// global operators for math
//

template <
    typename T,
    typename L,
    typename = typename std::enable_if<
        latex::math::can_solve<T>::value ||
        latex::math::can_solve<L>::value
    >::type
>
latex::math::Addition<T, L> operator+(const T& lhs, L rhs) { return latex::math::Addition<T, L>(lhs, rhs); }

//template <typename T, typename L, typename = typename std::enable_if<latex::math::can_solve<L>::value>::type>

template <
    typename T,
    typename L,
    typename = typename std::enable_if<
        latex::math::can_solve<T>::value ||
        latex::math::can_solve<L>::value
    >::type
>
latex::math::Subtraction<T, L> operator-(const T& lhs, L rhs) { return latex::math::Subtraction<T, L>(lhs, rhs); }

//template <typename T, typename L, typename = typename std::enable_if<latex::math::can_solve<L>::value>::type>

template <
    typename T,
    typename L,
    typename = typename std::enable_if<
        latex::math::can_solve<T>::value ||
        latex::math::can_solve<L>::value
    >::type
>
latex::math::Multiplication<T, L> operator*(const T& lhs, L rhs) { return latex::math::Multiplication<T, L>(lhs, rhs); }

//template <typename T, typename L, typename = typename std::enable_if<latex::math::can_solve<L>::value>::type>
template <
    typename T,
    typename L,
    typename = typename std::enable_if<
        latex::math::can_solve<T>::value ||
        latex::math::can_solve<L>::value
    >::type
>
latex::math::Fraction<T, L> operator/(const T& lhs, L rhs) { return latex::math::Fraction<T, L>(lhs, rhs); }

#endif
