#include "Database.h"
#include "CLI.h"

int main() {
    Database db;
    Parser parser;
    CLI cli(db, parser);
    cli.run();
    return 0;
}
