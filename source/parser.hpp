/*
 * parser.hpp
 *
 *  Created on: 08.04.2017
 *      Author: mgresens
 */

#pragma once

#include "idl.hpp"

namespace parser {

idl::Document
parse(const std::string& file);

}
