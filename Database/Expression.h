#ifndef EXPRESSION_H
#define EXPRESSION_H
#pragma once
#include "Prerequestion.h"


struct Expression {
    std::string column;
    std::string operators;
    std::string value;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    std::string logicalOperator; // "AND", "OR"
};

#endif // EXPRESSION_H