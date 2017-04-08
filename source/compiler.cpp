/*
 * compiler.cpp
 *
 *  Created on: 08.04.2017
 *      Author: mgresens
 */

#include "parser.hpp"
#include <iostream>

int main()
{
	try
	{
		parser::parse("idl/DocTest.thrift");
		parser::parse("idl/ThriftTest.thrift");
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << std::endl;
	}
	return 0;
}
