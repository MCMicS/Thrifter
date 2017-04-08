/*
 * idl.hpp
 *
 *  Created on: 08.04.2017
 *      Author: mgresens
 */

#pragma once

#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

namespace idl {

typedef std::string Identifier;
typedef std::string Literal;
typedef Literal Include;
typedef Literal CppInclude;

typedef boost::optional<std::string> Documentation;

enum class NamespaceScope
{
	all,
	cpp,
	java
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
	boost::optional<std::int32_t> value;
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
	string,
	//TODO
};

struct ListType;
struct SetType;
struct MapType;

typedef boost::variant<boost::recursive_wrapper<ListType>, boost::recursive_wrapper<SetType>, boost::recursive_wrapper<MapType>> ContainerType;

typedef boost::variant<Identifier, BaseType, ContainerType> FieldType;

typedef boost::optional<std::int32_t> FieldId;

struct Field
{
	Documentation documentation;
	FieldId id;
	FieldType type;
	Identifier identifier;
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

struct Throws
{
	Fields fields;
};

struct Function
{
	Documentation returns;
	FunctionType type;
	Documentation documentation;
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

typedef boost::variant<Enum, Struct, Service> Definition;

typedef std::vector<Definition> Definitions;

struct Document
{
	Documentation documentation;
	Headers headers;
	Definitions definitions;
};

}
