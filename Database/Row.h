#ifndef ROW_H
#define ROW_H
#pragma once

#include "Prerequestion.h"
#include "Column.h"

struct Row {


    Row(const std::vector<Column> &cols) : columns(&cols), columnsSet(cols.size(), false) {}

    std::vector<std::string> Data;
    std::vector<bool> columnsSet;
    const std::vector<Column> *columns;


    auto getValue(const std::string &columnName) -> std::string const;
    auto canUpdate(size_t columnIndex) -> bool;
    auto setValue(size_t columnIndex, const Row& inputRow) -> void;


};

#endif //ROW_H
