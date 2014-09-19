#include "parser.h"

#include <regex>
#include <string>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

// Debug
#include <iostream>

using namespace std;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

namespace {


	/*
	using qi::double_;
	using qi::_1;
	using qi::phrase_parse;
	using ascii::space;
	using phoenix::ref;

	template <typename Iterator>
	bool parse_numbers(Iterator first, Iterator last, double& result)
	{
		bool r = phrase_parse(
			first, 
			last,
			double_[ref(result) = _1] >> *(',' >> double_),
			space
		);

		if (first != last) // fail if we did not get a full match
			return false;

		return r;
	}
	*/

	/*
	template <typename Iterator>
	struct employee_parser : qi::grammar<Iterator, employee(), ascii::space_type>
	{
		employee_parser() : employee_parser::base_type(start)
		{
			using qi::int_;
			using qi::lit;
			using qi::double_;
			using qi::string_;
			using qi::lexeme;
			using ascii::char_;

			text %= lexeme['"' >> +(char_ - '"') >> '"'];

			start %=
				//lit("employee")
				string_
				>> '{'
				>>  int_ >> ','
				>>  quoted_string >> ','
				>>  quoted_string >> ','
				>>  double_
				>>  '}'
				;
		}

		//qi::rule<Iterator, std::string(), ascii::space_type> quoted_string;
		qi::rule<Iterator, employee(), ascii::space_type> start;
		qi::rule<Iterator, std::string(), ascii::space_type> text;
	};
	*/
}

namespace marklar {

	void parse(const string& str) {
		/*
			int main() {
				int i = 0;
				int j = 0;
				int r = i + j;

				return r;
			}
		*/

		cout << "Parsing: " << endl << str << endl;

		const regex programRegex("(\\S+)");
		auto words_begin = 
        std::sregex_iterator(str.begin(), str.end(), programRegex);
		auto words_end = std::sregex_iterator();
	 
		std::cout << "Found "
				  << std::distance(words_begin, words_end)
				  << " words\n";
	}

	void parseFunction(const string& str) {
		cout << "parseFunction:" << endl;

		auto rule = *(qi::lit("cat") [ ++qi::_val ] | qi::omit[qi::char_]);
		qi::parse(str.begin(), str.end(), rule, count);
	}

	double parseTest(const string& str) {
		double result = 0.0;
		/*
		const auto r = parse_numbers(str.begin(), str.end(), result);
		cout << "Parse numbers: " << r << endl;
		*/
		
		return result;
	}
}
