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
		parser::parse("../ThrifterOld/idl/MyService.thrift");
		parser::parse("../ThrifterOld/idl/DocTest.thrift");
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << std::endl;
	}
	return 0;
}
