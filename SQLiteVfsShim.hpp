#ifndef __SQLITE_VFS_SHIM_HPP__
#define __SQLITE_VFS_SHIM_HPP__

#include <sqlite3.h>

/**
 * SQLite File shim class with virtual methods for C++.
 *
 * @see https://sqlite.org/c3ref/file.html
 */
class SQLiteFileShim : sqlite3_file {
public:
	SQLiteFileShim();
	SQLiteFileShim(sqlite3_file *original_file);

	virtual int xClose();
	virtual int xRead(void *p, int iAmt, sqlite3_int64 iOfst);
	virtual int xWrite(const void *p, int iAmt, sqlite3_int64 iOfst);
	virtual int xTruncate(sqlite3_int64 size);
	virtual int xSync(int flags);
	virtual int xFileSize(sqlite3_int64 *pSize);
	virtual int xLock(int flags);
	virtual int xUnlock(int flags);
	virtual int xCheckReservedLock(int *pResOut);
	virtual int xFileControl(int op, void *pArg);
	virtual int xSectorSize();
	virtual int xDeviceCharacteristics();
	/* Methods above are valid for version 1 */
	virtual int xShmMap(int iPg, int pgsz, int, void volatile**pp);
	virtual int xShmLock(int offset, int n, int flags);
	virtual void xShmBarrier();
	virtual int xShmUnmap(int deleteFlag);
	/* Methods above are valid for version 2 */
	virtual int xFetch(sqlite3_int64 iOfst, int iAmt, void **pp);
	virtual int xUnfetch(sqlite3_int64 iOfst, void *p);
	/* Methods above are valid for version 3 */
	/* Additional methods may be added in future releases */

protected:
	sqlite3_file *original_file;
};

/**
 * SQLite VFS shim class with virtual methods for C++.
 *
 * @see https://sqlite.org/c3ref/vfs.html
 */
class SQLiteVfsShim : sqlite3_vfs {
public:
	SQLiteVfsShim(const char *name, const char *base_vfs_name = nullptr);
	SQLiteVfsShim(const char *name, int file_shim_size, const char *base_vfs_name = nullptr);
	
	int register_vfs(bool makeDefault);
	int unregister_vfs();

	template<class TFileShim>
	int open(sqlite3_filename zName, sqlite3_file *file, int flags, int *pOutFlags) {
		TFileShim *file_shim = (TFileShim *) file;
		sqlite3_file *original_file = (sqlite3_file *) (file_shim + 1);
		int result = original_vfs->xOpen(original_vfs, zName, original_file, flags, pOutFlags);
		if (result == SQLITE_OK) {
			*file_shim = TFileShim(original_file);
		}
		else {
			*file_shim = TFileShim();
		}
		return result;
	}

	virtual int xOpen(sqlite3_filename zName, sqlite3_file *file, int flags, int *pOutFlags);
	virtual int xDelete(const char *zName, int syncDir);
	virtual int xAccess(const char *zName, int flags, int *pResOut);
	virtual int xFullPathname(const char *zName, int nOut, char *zOut);
	virtual void *xDlOpen(const char *zFilename);
	virtual void xDlError(int nByte, char *zErrMsg);
	virtual void (*xDlSym(void *library, const char *zSymbol))(void);
	virtual void xDlClose(void *library);
	virtual int xRandomness(int nByte, char *zOut);
	virtual int xSleep(int microseconds);
	virtual int xCurrentTime(double *pResOut);
	virtual int xGetLastError(int nByte, char *zOut);
	/*
	** The methods above are in version 1 of the sqlite_vfs object
	** definition.  Those that follow are added in version 2 or later
	*/
	virtual int xCurrentTimeInt64(sqlite3_int64 *pResOut);
	/*
	** The methods above are in versions 1 and 2 of the sqlite_vfs object.
	** Those below are for version 3 and greater.
	*/
	virtual int xSetSystemCall(const char *zName, sqlite3_syscall_ptr ptr);
	virtual sqlite3_syscall_ptr xGetSystemCall(const char *zName);
	virtual const char *xNextSystemCall(const char *zName);
	/*
	** The methods above are in versions 1 through 3 of the sqlite_vfs object.
	** New fields may be appended in future versions.  The iVersion
	** value will increment whenever this happens.
	*/
	
protected:
	/// Base VFS used by this shim.
	/// Used by any methods you do not implement.
	sqlite3_vfs *original_vfs;
};

#endif

///////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////
#ifdef SQLITE_VFS_SHIM_IMPLEMENTATION

template<auto SQLiteFileShim::*fptr, typename... Args>
static auto wrap_file_method(sqlite3_file *file, Args... args) {
	return (((SQLiteFileShim *) file)->*fptr)(args...);
}

static sqlite3_io_methods SQLiteFileShimMethods = {
  3,
  wrap_file_method<&SQLiteFileShim::xClose>,
  wrap_file_method<&SQLiteFileShim::xRead>,
  wrap_file_method<&SQLiteFileShim::xWrite>,
  wrap_file_method<&SQLiteFileShim::xTruncate>,
  wrap_file_method<&SQLiteFileShim::xSync>,
  wrap_file_method<&SQLiteFileShim::xFileSize>,
  wrap_file_method<&SQLiteFileShim::xLock>,
  wrap_file_method<&SQLiteFileShim::xUnlock>,
  wrap_file_method<&SQLiteFileShim::xCheckReservedLock>,
  wrap_file_method<&SQLiteFileShim::xFileControl>,
  wrap_file_method<&SQLiteFileShim::xSectorSize>,
  wrap_file_method<&SQLiteFileShim::xDeviceCharacteristics>,
  wrap_file_method<&SQLiteFileShim::xShmMap>,
  wrap_file_method<&SQLiteFileShim::xShmLock>,
  wrap_file_method<&SQLiteFileShim::xShmBarrier>,
  wrap_file_method<&SQLiteFileShim::xShmUnmap>,
  wrap_file_method<&SQLiteFileShim::xFetch>,
  wrap_file_method<&SQLiteFileShim::xUnfetch>,
};

// File
SQLiteFileShim::SQLiteFileShim()
	: sqlite3_file()
{
}
SQLiteFileShim::SQLiteFileShim(sqlite3_file *original_file)
	: sqlite3_file({ &SQLiteFileShimMethods }), original_file(original_file)
{
}

int SQLiteFileShim::xClose() {
	return original_file->pMethods->xClose(original_file);
}
int SQLiteFileShim::xRead(void *p, int iAmt, sqlite3_int64 iOfst) {
	return original_file->pMethods->xRead(original_file, p, iAmt, iOfst);
}
int SQLiteFileShim::xWrite(const void *p, int iAmt, sqlite3_int64 iOfst) {
	return original_file->pMethods->xWrite(original_file, p, iAmt, iOfst);
}
int SQLiteFileShim::xTruncate(sqlite3_int64 size) {
	return original_file->pMethods->xTruncate(original_file, size);
}
int SQLiteFileShim::xSync(int flags) {
	return original_file->pMethods->xSync(original_file, flags);
}
int SQLiteFileShim::xFileSize(sqlite3_int64 *pSize) {
	return original_file->pMethods->xFileSize(original_file, pSize);
}
int SQLiteFileShim::xLock(int flags) {
	return original_file->pMethods->xLock(original_file, flags);
}
int SQLiteFileShim::xUnlock(int flags) {
	return original_file->pMethods->xUnlock(original_file, flags);
}
int SQLiteFileShim::xCheckReservedLock(int *pResOut) {
	return original_file->pMethods->xCheckReservedLock(original_file, pResOut);
}
int SQLiteFileShim::xFileControl(int op, void *pArg) {
	return original_file->pMethods->xFileControl(original_file, op, pArg);
}
int SQLiteFileShim::xSectorSize() {
	return original_file->pMethods->xSectorSize(original_file);
}
int SQLiteFileShim::xDeviceCharacteristics() {
	return original_file->pMethods->xDeviceCharacteristics(original_file);
}
int SQLiteFileShim::xShmMap(int iPg, int pgsz, int flags, void volatile **pp) {
	return original_file->pMethods->xShmMap(original_file, iPg, pgsz, flags, pp);
}
int SQLiteFileShim::xShmLock(int offset, int n, int flags) {
	return original_file->pMethods->xShmLock(original_file, offset, n, flags);
}
void SQLiteFileShim::xShmBarrier() {
	return original_file->pMethods->xShmBarrier(original_file);
}
int SQLiteFileShim::xShmUnmap(int deleteFlag) {
	return original_file->pMethods->xShmUnmap(original_file, deleteFlag);
}
int SQLiteFileShim::xFetch(sqlite3_int64 iOfst, int iAmt, void **pp) {
	return original_file->pMethods->xFetch(original_file, iOfst, iAmt, pp);
}
int SQLiteFileShim::xUnfetch(sqlite3_int64 iOfst, void *p) {
	return original_file->pMethods->xUnfetch(original_file, iOfst, p);
}

// VFS
template<auto SQLiteVfsShim::*fptr, typename... Args>
static auto wrap_vfs_method(sqlite3_vfs *vfs, Args... args) {
	return (((SQLiteVfsShim *) vfs)->*fptr)(args...);
}

SQLiteVfsShim::SQLiteVfsShim(const char *name, const char *base_vfs_name)
	: SQLiteVfsShim(name, sizeof(SQLiteFileShim), base_vfs_name)
{
}
SQLiteVfsShim::SQLiteVfsShim(const char *name, int file_shim_size, const char *base_vfs_name) {
	original_vfs = sqlite3_vfs_find(base_vfs_name) ?: sqlite3_vfs_find(nullptr);
	iVersion = original_vfs->iVersion;
	szOsFile = (int) file_shim_size + original_vfs->szOsFile;
	mxPathname = original_vfs->mxPathname;
	pNext = nullptr;
	zName = name;
	pAppData = nullptr;
	sqlite3_vfs::xOpen = wrap_vfs_method<&SQLiteVfsShim::xOpen>;
	sqlite3_vfs::xDelete = wrap_vfs_method<&SQLiteVfsShim::xDelete>;
	sqlite3_vfs::xAccess = wrap_vfs_method<&SQLiteVfsShim::xAccess>;
	sqlite3_vfs::xFullPathname = wrap_vfs_method<&SQLiteVfsShim::xFullPathname>;
	sqlite3_vfs::xDlOpen = wrap_vfs_method<&SQLiteVfsShim::xDlOpen>;
	sqlite3_vfs::xDlError = wrap_vfs_method<&SQLiteVfsShim::xDlError>;
	sqlite3_vfs::xDlSym = wrap_vfs_method<&SQLiteVfsShim::xDlSym>;
	sqlite3_vfs::xDlClose = wrap_vfs_method<&SQLiteVfsShim::xDlClose>;
	sqlite3_vfs::xRandomness = wrap_vfs_method<&SQLiteVfsShim::xRandomness>;
	sqlite3_vfs::xSleep = wrap_vfs_method<&SQLiteVfsShim::xSleep>;
	sqlite3_vfs::xCurrentTime = wrap_vfs_method<&SQLiteVfsShim::xCurrentTime>;
	sqlite3_vfs::xGetLastError = wrap_vfs_method<&SQLiteVfsShim::xGetLastError>;
	sqlite3_vfs::xCurrentTimeInt64 = wrap_vfs_method<&SQLiteVfsShim::xCurrentTimeInt64>;
	sqlite3_vfs::xSetSystemCall = wrap_vfs_method<&SQLiteVfsShim::xSetSystemCall>;
	sqlite3_vfs::xGetSystemCall = wrap_vfs_method<&SQLiteVfsShim::xGetSystemCall>;
	sqlite3_vfs::xNextSystemCall = wrap_vfs_method<&SQLiteVfsShim::xNextSystemCall>;
}

int SQLiteVfsShim::register_vfs(bool makeDefault) {
	return sqlite3_vfs_register(this, makeDefault);
}

int SQLiteVfsShim::unregister_vfs() {
	return sqlite3_vfs_unregister(this);
}

int SQLiteVfsShim::xOpen(sqlite3_filename zName, sqlite3_file *file, int flags, int *pOutFlags) {
	return open<SQLiteFileShim>(zName, file, flags, pOutFlags);
}
int SQLiteVfsShim::xDelete(const char *zName, int syncDir) {
	return original_vfs->xDelete(original_vfs, zName, syncDir);
}
int SQLiteVfsShim::xAccess(const char *zName, int flags, int *pResOut) {
	return original_vfs->xAccess(original_vfs, zName, flags, pResOut);
}
int SQLiteVfsShim::xFullPathname(const char *zName, int nOut, char *zOut) {
	return original_vfs->xFullPathname(original_vfs, zName, nOut, zOut);
}
void *SQLiteVfsShim::xDlOpen(const char *zFilename) {
	return original_vfs->xDlOpen(original_vfs, zFilename);
}
void SQLiteVfsShim::xDlError(int nByte, char *zErrMsg) {
	original_vfs->xDlError(original_vfs, nByte, zErrMsg);
}
void (*SQLiteVfsShim::xDlSym(void *library, const char *zSymbol))(void) {
	return original_vfs->xDlSym(original_vfs, library, zSymbol);
}
void SQLiteVfsShim::xDlClose(void *library) {
	return original_vfs->xDlClose(original_vfs, library);
}
int SQLiteVfsShim::xRandomness(int nByte, char *zOut) {
	return original_vfs->xRandomness(original_vfs, nByte, zOut);
}
int SQLiteVfsShim::xSleep(int microseconds) {
	return original_vfs->xSleep(original_vfs, microseconds);
}
int SQLiteVfsShim::xCurrentTime(double *pResOut) {
	return original_vfs->xCurrentTime(original_vfs, pResOut);
}
int SQLiteVfsShim::xGetLastError(int nByte, char *zOut) {
	return original_vfs->xGetLastError(original_vfs, nByte, zOut);
}
int SQLiteVfsShim::xCurrentTimeInt64(sqlite3_int64 *pResOut) {
	return original_vfs->xCurrentTimeInt64(original_vfs, pResOut);
}
int SQLiteVfsShim::xSetSystemCall(const char *zName, sqlite3_syscall_ptr ptr) {
	return original_vfs->xSetSystemCall(original_vfs, zName, ptr);
}
sqlite3_syscall_ptr SQLiteVfsShim::xGetSystemCall(const char *zName) {
	return original_vfs->xGetSystemCall(original_vfs, zName);
}
const char *SQLiteVfsShim::xNextSystemCall(const char *zName) {
	return original_vfs->xNextSystemCall(original_vfs, zName);
}

#endif