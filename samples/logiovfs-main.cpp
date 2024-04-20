#include <iostream>

#include <sqlite3.h>

using namespace std;

int main(int argc, const char **argv) {
	sqlite3 *db = nullptr;
	int result = sqlite3_open("", &db);
	if (result != SQLITE_OK) {
		cout << "Error: " << sqlite3_errstr(result) << endl;
		return result;
	}

	result = sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, nullptr);
	if (result != SQLITE_OK) {
		cout << "Error: " << sqlite3_errstr(result) << endl;
		return result;
	}

	char *err = nullptr;
	result = sqlite3_load_extension(db, "logiovfs", nullptr, &err);
	if (result != SQLITE_OK && result != SQLITE_OK_LOAD_PERMANENTLY) {
		cout << "Error: " << (err ?: sqlite3_errstr(result)) << endl;
		sqlite3_free(err);
		sqlite3_close(db);
		return result;
	}

	sqlite3_close(db);

	const char *db_name = argc > 1 ? argv[1] : "testdb.sqlite";
	result = sqlite3_open_v2(db_name, &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, "logiovfs");
	if (result != SQLITE_OK) {
		cout << "Error: " << sqlite3_errstr(result) << endl;
		return result;
	}
	result = sqlite3_exec(db, "VACUUM", nullptr, nullptr, &err);
	if (result != SQLITE_OK) {
		cout << "Error: " << (err ?: sqlite3_errstr(result)) << endl;
		sqlite3_free(err);
	}
	sqlite3_close(db);
	return 0;
}