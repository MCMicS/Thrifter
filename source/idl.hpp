/*
 * idl.hpp
 *
 *  Created on: 08.04.2017
 *      Author: mgresens
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

namespace idl {

typedef std::string Identifier;
typedef std::string Literal;
typedef Literal Include;
typedef Literal CppInclude;

typedef std::int64_t IntConstant;
typedef double DoubleConstant;

typedef boost::make_recursive_variant<
			Identifier,
			Literal,
			IntConstant,
			DoubleConstant,
			std::vector<boost::recursive_variant_>,
			std::map<boost::recursive_variant_, boost::recursive_variant_>
>::type ConstValue;

typedef std::vector<ConstValue> ConstList;
typedef std::map<ConstValue, ConstValue> ConstMap;

typedef boost::optional<std::string> Documentation;

enum class NamespaceScope
{
	all,
	cpp,
	java,
	py,
	perl,
	rb,
	cocoa,
	csharp,
	c_glib,
	js,
	st
	// TODO
};

struct Namespace
{
	NamespaceScope scope;
	Identifier identifier;
};

typedef boost::variant<Include, CppInclude, Namespace> Header;

typedef std::vector<Header> Headers;

struct Enumerator
{
	Documentation documentation;
	Identifier identifier;
	boost::optional<IntConstant> value;
};

typedef std::vector<Enumerator> Enumerators;

struct Enum
{
	Documentation documentation;
	Identifier identifier;
	Enumerators enumerators;
};

enum class BaseType
{
	bool_,
	i8,
	i16,
	i32,
	i64,
	double_,
	string,
	binary
	//TODO
};

struct ListType;
struct SetType;
struct MapType;

typedef boost::variant<boost::recursive_wrapper<ListType>, boost::recursive_wrapper<SetType>, boost::recursive_wrapper<MapType>> ContainerType;

typedef boost::variant<Identifier, BaseType, ContainerType> FieldType;

typedef boost::optional<IntConstant> FieldId;

struct Field
{
	Documentation documentation;
	FieldId id;
	bool optional;
	FieldType type;
	Identifier identifier;
	boost::optional<ConstValue> value;
};

typedef std::vector<Field> Fields;

struct Parameter
{
	Documentation documentation;
	FieldId id;
	FieldType type;
	Identifier identifier;
};

typedef std::vector<Parameter> Parameters;

struct ListType
{
	FieldType type;
};

struct SetType
{
	FieldType type;
};

struct MapType
{
	FieldType key;
	FieldType value;
};

struct VoidType
{
};

typedef boost::variant<FieldType, VoidType> FunctionType;

struct Struct
{
	Documentation documentation;
	Identifier identifier;
	Fields fields;
};

struct Union
{
	Documentation documentation;
	Identifier identifier;
	Fields fields;
};

struct Exception
{
	Documentation documentation;
	Identifier identifier;
	Fields fields;
};

struct Throws
{
	Fields fields;
};

struct Function
{
//	Documentation returns;
	Documentation documentation;
	bool oneway;
	FunctionType type;
	Identifier identifier;
	Parameters parameters;
	Throws throws;
};

typedef std::vector<Function> Functions;

struct Service
{
	Documentation documentation;
	Identifier identifier;
	Functions functions;
};

struct Typedef
{
	Documentation documentation;
	FieldType type;
	Identifier identifier;
};

struct Const
{
	Documentation documentation;
	FieldType type;
	Identifier identifier;
	ConstValue value;
};

typedef boost::variant<Const, Typedef, Enum, Struct, Union, Exception, Service> Definition;

typedef std::vector<Definition> Definitions;

struct Document
{
	Documentation documentation;
	Headers headers;
	Definitions definitions;
};

}
