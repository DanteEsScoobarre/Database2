//
// Created by rozsh on 17/01/2024.
//

#ifndef DATABASE2_ROW_H
#define DATABASE2_ROW_H
#pragma once

#include "Prerequestion.h"
#include "Column.h"

struct Row {
    Row() : columns(nullptr) {}

    Row(const std::vector<Column> &cols) : columns(&cols) {}

    std::vector<std::string> Data;
    const std::vector<Column> *columns;


    auto getValue(const std::string &columnName) -> std::string const;


};

#endif //DATABASE2_ROW_H
