// 1. Include <SQLiteVfs.hpp> header
// If you are building an extension DLL, make sure to include
// <sqlite3ext.h> and call SQLITE_EXTENSION_INIT1 first.
#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1
#include <SQLiteVfs.hpp>

#include <iostream>

using namespace sqlitevfs;
using namespace std;

// 2. Implement your own `SQLiteFileImpl` subclass.
// Override any IO methods necessary. Reference: https://www.sqlite.org/c3ref/io_methods.html
// Default implementation will forward execution to the `original_file` opened by `SQLiteVfsImpl::xOpen`.
// Default constructor will be called before `SQLiteVfsImpl::xOpen`.
// Destructor will be called right after `xClose`, or after a failed `SQLiteVfsImpl::xOpen`.
struct LogIOFileShim : public SQLiteFileImpl {
	LogIOFileShim() {
		cout << "> Constructing file!" << endl;
	}
	int xRead(void *p, int iAmt, sqlite3_int64 iOfst) override {
		cout << "> READ " << iAmt << " bytes starting at " << iOfst << endl;
		return SQLiteFileImpl::xRead(p, iAmt, iOfst);
	}
	int xWrite(const void *p, int iAmt, sqlite3_int64 iOfst) override {
		cout << "> WRITE " << iAmt << " bytes starting at " << iOfst << endl;
		return SQLiteFileImpl::xWrite(p, iAmt, iOfst);
	}
	int xClose() override {
		cout << "> CLOSE" << endl;
		return SQLiteFileImpl::xClose();
	}
	~LogIOFileShim() {
		cout << "> Destroying file!" << endl;
	}
};

// 3. Implement your own `SQLiteVfsImpl<>` subclass.
// Pass your SQLiteFileImpl subclass as template parameter.
// Override any methods necessary. Reference: https://www.sqlite.org/c3ref/vfs.html
// Default implementation will forward execution to the `original_vfs` passed in `SQLiteVfs` construtor.
// Notice that `xOpen` receives a `SQLiteFile<LogIOFileImpl> *` instead of `sqlite3_file`.
// `file` is guaranteed to be fully constructed before this method is called.
struct LogIOVfsShim : public SQLiteVfsImpl<LogIOFileShim> {
	int xOpen(sqlite3_filename zName, SQLiteFile<LogIOFileShim> *file, int flags, int *pOutFlags) override {
		int result = SQLiteVfsImpl::xOpen(zName, file, flags, pOutFlags);
		if (result == SQLITE_OK) {
			cout << "> OPENED '" << zName << "'" << endl;
		}
		else {
			cout << "> ERROR OPENING '" << zName << "': " << sqlite3_errstr(result) << endl;
		}
		return result;
	}
};

// If implementing an extension DLL, export the function SQLite expects
extern "C" int sqlite3_logiovfs_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
	SQLITE_EXTENSION_INIT2(pApi);

	// 4. Create a `SQLiteVfs<>`.
	// Pass your `SQLiteVfsImpl<>` subclass as template parameter.
	static SQLiteVfs<LogIOVfsShim> logiovfs("logiovfs");

	// 5. Register your newly created VFS.
	// Optionally make it the default VFS.
	int rc = logiovfs.register_vfs(false);
	if (rc == SQLITE_OK) {
		rc = SQLITE_OK_LOAD_PERMANENTLY;
	}
	return rc;
}

// 6. (optional) Unregister your VFS using `your_vfs.unregister_vfs()`