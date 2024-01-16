#include "Database.h"
#include "CLI.h"

int main() {
    Database db;
    CLI cli(db);
    cli.run();
    return 0;
}
