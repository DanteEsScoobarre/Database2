//
// Created by rozsh on 17/01/2024.
//

#ifndef DATABASE2_ROW_H
#define DATABASE2_ROW_H
#pragma once

#include "Prerequestion.h"
#include "Column.h"

struct Row {


    Row(const std::vector<Column> &cols) : columns(&cols), columnsSet(cols.size(), false) {}

    std::vector<std::string> Data;
    std::vector<bool> columnsSet;
    const std::vector<Column> *columns;


    auto getValue(const std::string &columnName) -> std::string const;

    auto setColumns(const std::vector<Column> &cols) -> void;


    bool isColumnSet(size_t index);

    auto canUpdate(size_t columnIndex) -> bool;

    auto setValue(size_t columnIndex, const Row& inputRow) -> void;


};

#endif //DATABASE2_ROW_H
