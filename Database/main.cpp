#include "Database.h"
#include "CLI.h"

/*
 *
 * Dla lepszych wrażeń związanych z korzystaniem z bazy danych polecam włączyć ją w innym oknie niż konsola IDE.
 *
 *
 Dane przechowywane są w kolumnach i rzędach, które zawierają nazwę kolumny oraz typ danych jakie przechowują. Następnie kolumny
 tworzą rzędy będące vectorem kolumn. Dopóki użytkownik nie wprowadzi komendy SAVE wszystkie dane są przechowywane w polach
 klas. Dodatkowo, wszystkie komendy są parsowane w parserzem który następnie wysyła odpowiednie tokeny do funckji która się wykonuje.
 Niestety, z jakiegoś

 Klasa Database zawiera wszystkie metody, niezbędne do tworzenia tabel, dodawania do nich kolumn, usuwania kolumn,
 wprowadzania danych do tabeli, usuwania danych z tabeli, wyświetlania danych z tabel oraz modyfikacją danych w tabeli.

 Klasa CLI odpowiada za kontakt użytkownika z programem poprzez prosty interfejs.

 Klasa Parser odpowiada za przetwarzanie komend podanych przez użytkownika poprzez dzielenie komendy na tokeny,
 które następnie są wprowadzane do funkcji parsującej która na podstawie słów kluczowych uruchamia odpowiednie metody.

 Klasa FileOps odpowiada głównie za tworzenie backupu bazy danych oraz wczytaniem backupu bazy danych z pliku.




 Użytkownik jest w stanie wykorzystać każdą wymienioną komendę do wywołania funkcji programu.


 Składnia do wywołania każdej funckcji programu.

 Dla CREATE - tworzy nową tabelę.
 CREATE table_name WITH {column_name, data_type}

 Dla ADD - dodaje nową kolumnę do tabeli.
 ADD {column_name, data_type} INTO table_name

 Dla INSERT - wprowadzanie danych do tabeli
 INSERT [int] INTO column_name IN table_name
 INSERT ['string'] INTO column_name IN table_name
 INSERT [(boolean)] INTO column_name IN table_name

 Dla SELECT
 SELECT column_name FROM table_name WHERE condition

 Dla UPDATE - zmiany danych w tabeli
 UPDATE column_name FROM table_name WITH [updated_value]

 Dla DELETE - usuwanie danych z tabeli
 DELETE [data] FROM column_name IN table_name

 Dla REMOVE - usuwanie column z tabeli
 REMOVE column_name FROM table_name

 Dla DROP - usuwanie tabeli
 DROP table_name

 Dla SAVE - zapisanie danych do pliku
 SAVE absolute_path_to_file

 Dla LOAD - wczytywanie danych z pliku
 LOAD absolute_path_to_file

Mimo, że nie korzystam z SFML w aplikacji, ale jak go nie ma to się aplikacja nie kompiluje, prawdopobnie jest to związane z CLionem i plikami w debugCmakee, ale zostawiamn na wszelki wypadek.

*/
int main() {
    Database db;
    Parser parser;
    CLI cli(db, parser);
    cli.run();
    return 0;
}
