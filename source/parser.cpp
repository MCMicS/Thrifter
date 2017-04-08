/*
 * parser.cpp
 *
 *  Created on: 08.04.2017
 *      Author: mgresens
 */

#include "parser.hpp"
#include <fstream>
#include <unordered_map>
#include <boost/optional/optional_io.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>

namespace x3 = boost::spirit::x3;

BOOST_FUSION_ADAPT_STRUCT(idl::Namespace, scope, identifier);
BOOST_FUSION_ADAPT_STRUCT(idl::Enumerator, documentation, identifier, value);
BOOST_FUSION_ADAPT_STRUCT(idl::Enum, documentation, identifier, enumerators);
BOOST_FUSION_ADAPT_STRUCT(idl::ListType, type);
BOOST_FUSION_ADAPT_STRUCT(idl::SetType, type);
BOOST_FUSION_ADAPT_STRUCT(idl::MapType, key, value);
BOOST_FUSION_ADAPT_STRUCT(idl::VoidType);
BOOST_FUSION_ADAPT_STRUCT(idl::Field, documentation, id, optional, type, identifier, value);
BOOST_FUSION_ADAPT_STRUCT(idl::Parameter, documentation, id, type, identifier);
BOOST_FUSION_ADAPT_STRUCT(idl::Struct, documentation, identifier, fields);
BOOST_FUSION_ADAPT_STRUCT(idl::Exception, documentation, identifier, fields);
BOOST_FUSION_ADAPT_STRUCT(idl::Throws, fields);
//BOOST_FUSION_ADAPT_STRUCT(idl::Function, returns, type, documentation, identifier, parameters, throws);
BOOST_FUSION_ADAPT_STRUCT(idl::Function, documentation, oneway, type, identifier, parameters, throws);
BOOST_FUSION_ADAPT_STRUCT(idl::Service, documentation, identifier, functions);
BOOST_FUSION_ADAPT_STRUCT(idl::Document, documentation, headers, definitions);
BOOST_FUSION_ADAPT_STRUCT(idl::Typedef, documentation, type, identifier);
BOOST_FUSION_ADAPT_STRUCT(idl::Const, documentation, type, identifier, value);

namespace parser {

const x3::rule<struct Comment> comment("Comment");
const x3::rule<struct LineComment> lineComment("LineComment");
const x3::rule<struct BlockComment> blockComment("BlockComment");
const x3::rule<struct Documentation, idl::Documentation> documentation("Documentation");
const x3::rule<struct LineDocumentation, std::string> lineDocumentation("LineDocumentation");
const x3::rule<struct BlockDocumentation, std::string> blockDocumentation("BlockDocumentation");
const x3::rule<struct Identifier, idl::Identifier> identifier("Identifier");
const x3::rule<struct Literal, idl::Literal> literal("Literal");
const x3::rule<struct FieldId, idl::FieldId> fieldId("FieldId");
const x3::rule<struct FieldType, idl::FieldType> fieldType("FieldType");
const x3::rule<struct ListType, idl::ListType> listType("ListType");
const x3::rule<struct SetType, idl::SetType> setType("SetType");
const x3::rule<struct MapType, idl::MapType> mapType("MapType");
const x3::rule<struct ContainerType, idl::ContainerType> containerType("ContainerType");
const x3::rule<struct BaseType, idl::BaseType> baseType("BaseType");
const x3::rule<struct VoidType, idl::VoidType> voidType("VoidType");
const x3::rule<struct FunctionType, idl::FunctionType> functionType("FunctionType");
const x3::rule<struct Field, idl::Field> field("Field");
const x3::rule<struct Fields, idl::Fields> fields("Fields");
const x3::rule<struct Parameter, idl::Parameter> parameter("Parameter");
const x3::rule<struct Function, idl::Function> function("Function");
const x3::rule<struct Throws, idl::Throws> throws("Throws");
const x3::rule<struct Service, idl::Service> service("Service");
const x3::rule<struct Struct, idl::Struct> struct_("Struct");
const x3::rule<struct Exception, idl::Exception> exception("Exception");
const x3::rule<struct Enumerator, idl::Enumerator> enumerator("Enumerator");
const x3::rule<struct Enum, idl::Enum> enum_("Enum");
const x3::rule<struct Definition, idl::Definition> definition("Definition");
const x3::rule<struct Definitions, idl::Definitions> definitions("Definitions");
const x3::rule<struct NamespaceScope, idl::NamespaceScope> namespaceScope("NamespaceScope");
const x3::rule<struct Namespace, idl::Namespace> namespace_("Namespace");
const x3::rule<struct CppInclude, idl::CppInclude> cppInclude("CppInclude");
const x3::rule<struct Include, idl::Include> include("Include");
const x3::rule<struct Header, idl::Header> header("Header");
const x3::rule<struct Headers, idl::Headers> headers("Headers");
const x3::rule<struct Document, idl::Document> document("Document");
const x3::rule<struct Typedef, idl::Typedef> typedef_("Typedef");
const x3::rule<struct Const, idl::Const> const_("Const");
const x3::rule<struct ConstValue, idl::ConstValue> constValue("ConstValue");
const x3::rule<struct ListSeparator> listSeparator("ListSeparator");
const x3::rule<struct FieldReq, bool> fieldReq("FieldReq");
const x3::rule<struct Oneway, bool> oneway("Oneway");

const auto listSeparator_def = x3::lit(',') | x3::lit(';');
const auto comment_def = lineComment | blockComment;
const auto lineComment_def = ((x3::lit("//") - x3::lit("///")) | x3::lit('#')) >> *(x3::char_ - x3::eol) >> x3::eol;
const auto blockComment_def = (x3::lit("/*") - x3::lit("/**")) >> *(x3::char_ - x3::lit("*/")) >> x3::lit("*/");
const auto documentation_def = lineDocumentation | blockDocumentation;
const auto lineDocumentation_def = x3::lexeme[x3::lit("///") >> *(x3::char_ - x3::eol) >> x3::eol];
const auto blockDocumentation_def = x3::lexeme[x3::lit("/**") >> *(x3::char_ - x3::lit("*/")) >> x3::lit("*/")];
const auto identifier_def = x3::lexeme[(x3::alpha | x3::char_('_')) > *(x3::alnum | x3::char_('.') | x3::char_('_'))];
const auto literal_def = x3::lexeme[x3::lit('"') > *(x3::char_ - x3::lit('"')) > x3::lit('"')];
const auto listType_def = x3::lit("list") > x3::lit('<') > fieldType > x3::lit('>');
const auto setType_def = x3::lit("set") > x3::lit('<') > fieldType > x3::lit('>');
const auto mapType_def = x3::lit("map") > x3::lit('<') > fieldType > x3::lit(',') > fieldType > x3::lit('>');
const auto containerType_def = listType | setType | mapType;
const auto baseType_def = x3::symbols<idl::BaseType>
{
	{"bool", idl::BaseType::bool_},
	{"byte", idl::BaseType::i8},
	{"i8", idl::BaseType::i8},
	{"i16", idl::BaseType::i16},
	{"i32", idl::BaseType::i32},
	{"i64", idl::BaseType::i64},
	{"double", idl::BaseType::double_},
	{"string", idl::BaseType::string},
	{"binary", idl::BaseType::binary}
};
const auto fieldReq_def = x3::symbols<bool>
{
	{"required", false},
	{"optional", true}
};
const auto fieldType_def = containerType | baseType | identifier;
const auto voidType_def = x3::lit("void") >> x3::attr(idl::VoidType());
const auto oneway_def = x3::lit("oneway") >> x3::attr(true);
const auto functionType_def = voidType | fieldType;
const auto throws_def = x3::lit("throws") > x3::lit('(') > fields > x3::lit(')');
//const auto throws_def = x3::lit("throws") - x3::lit(';') > x3::lit('(') > field % x3::lit(',') > x3::lit(')');

const auto function_def = (-documentation >> -oneway >> functionType) > identifier > x3::lit('(') > *parameter > x3::lit(')') > -throws > -listSeparator;

//const auto function_def = (-documentation >> functionType) > (-documentation >> identifier) > x3::lit('(') > parameter % x3::lit(',') > x3::lit(')') > -throws;
const auto fieldId_def = x3::int_ > x3::lit(':');
const auto fields_def = *field;
const auto field_def = (-documentation >> -fieldId >> -fieldReq >> fieldType) > identifier > -(x3::lit('=') > constValue) > -listSeparator;
const auto parameter_def = (-documentation >> -fieldId >> fieldType) > identifier > -listSeparator;

const auto service_def = (-documentation >> x3::lit("service")) > identifier > x3::lit('{') > *function > x3::lit('}');

const auto struct__def = (-documentation >> x3::lit("struct")) > identifier > x3::lit('{') > fields > x3::lit('}');
const auto exception_def = (-documentation >> x3::lit("exception")) > identifier > x3::lit('{') > fields > x3::lit('}');
//const auto struct__def = (-documentation >> x3::lit("struct")) > identifier > x3::lit('{') > *((field - x3::lit('}')) > x3::lit(';')) > x3::lit('}');
const auto enumerator_def = (-documentation >> identifier) > -(x3::lit('=') > x3::int32) > -listSeparator;
const auto enum__def = (-documentation >> x3::lit("enum")) > identifier > x3::lit('{') > *enumerator > x3::lit('}');

const auto namespaceScope_def = x3::symbols<idl::NamespaceScope>
{
	{"*", idl::NamespaceScope::all},
	{"cpp", idl::NamespaceScope::cpp},
	{"java", idl::NamespaceScope::java},
	{"py", idl::NamespaceScope::py},
	{"perl", idl::NamespaceScope::perl},
	{"rb", idl::NamespaceScope::rb},
	{"cocoa", idl::NamespaceScope::cocoa},
	{"csharp", idl::NamespaceScope::csharp},
	{"c_glib", idl::NamespaceScope::c_glib},
	{"js", idl::NamespaceScope::js},
	{"st", idl::NamespaceScope::st}
};
const auto namespace__def = x3::lit("namespace") > namespaceScope > identifier;
const auto cppInclude_def = x3::lit("cpp_include") > literal;
const auto include_def = x3::lit("include") > literal;
const auto header_def = include | cppInclude | namespace_;
const auto headers_def = *header;
const auto definition_def = const_ | typedef_ | enum_ | struct_ | exception | service;
const auto definitions_def = *definition;
const auto document_def = -documentation > headers > definitions;
const auto typedef__def = (-documentation >> x3::lit("typedef")) > fieldType > identifier;
const auto const__def = (-documentation >> x3::lit("const")) > fieldType > identifier > x3::lit('=') > constValue > -listSeparator;
const auto constValue_def = identifier | literal | x3::int32 | x3::double_;

BOOST_SPIRIT_DEFINE(identifier, literal, listType, setType, mapType);
BOOST_SPIRIT_DEFINE(containerType, baseType, fieldId, fieldType, voidType, functionType);
BOOST_SPIRIT_DEFINE(parameter, function, field, service, struct_, enumerator, enum_, throws);
BOOST_SPIRIT_DEFINE(definition, namespaceScope, namespace_);
BOOST_SPIRIT_DEFINE(cppInclude, include, header, document);
BOOST_SPIRIT_DEFINE(comment, lineComment, blockComment);
BOOST_SPIRIT_DEFINE(documentation, lineDocumentation,  blockDocumentation);
BOOST_SPIRIT_DEFINE(headers, definitions, typedef_, listSeparator, fields, const_, constValue, exception, fieldReq, oneway);

idl::Document
parse(const std::string& file)
{
	namespace spirit = boost::spirit;

	typedef spirit::line_pos_iterator<spirit::istream_iterator> iterator_t;

	std::ifstream in(file);
	in.unsetf(std::ios::skipws);
	idl::Document out;

	iterator_t iterator((iterator_t::base_type(in)));
	try
	{
		const iterator_t end;
		const auto skipper = x3::space | comment;

		const bool ok = x3::phrase_parse(iterator, end, document, skipper, out);

		if (!ok || iterator != end)
		{
			const std::size_t line = spirit::get_line(iterator);
			const std::size_t column = spirit::get_column(iterator, iterator_t());
			throw std::runtime_error("Parsing failed: " + std::to_string(line) + ',' + std::to_string(column));
		}
	}
	catch (const x3::expectation_failure<iterator_t>& exception)
	{
		const std::size_t line = spirit::get_line(exception.where());
		const std::size_t column = spirit::get_column(iterator, exception.where());
		throw std::runtime_error(exception.which() + " expected: " + std::to_string(line) + ',' + std::to_string(column));
	}

	return out;
}

}
