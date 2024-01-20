//
// Created by rozsh on 17/01/2024.
//

#ifndef DATABASE2_TABLE_H
#define DATABASE2_TABLE_H
#pragma once
#include "Prerequestion.h"
#include "Column.h"
#include "Row.h"

struct Table {
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;

};
#endif //DATABASE2_TABLE_H
