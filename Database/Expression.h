#ifndef EXPRESSION_H
#define EXPRESSION_H
#pragma once
#include "Prerequestion.h"

/*
 * Wiedzę o smart pointerach zarczerpnąłem stąd:
https://learn.microsoft.com/en-us/cpp/cpp/smart-pointers-modern-cpp?view=msvc-170
*/
 struct Expression {
    std::string column;
    std::string operators;
    std::string value;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    std::string logicalOperator;
};

#endif // EXPRESSION_H