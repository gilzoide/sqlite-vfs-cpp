#ifndef __SQLITE_VFS_HPP__
#define __SQLITE_VFS_HPP__

#include <sqlite3.h>

/**
 * SQLite File class with virtual methods for C++.
 *
 * By default, it forwards every call to the `original_file` passed to it in its constructor.
 *
 * @see https://sqlite.org/c3ref/file.html
 */
class SQLiteFile : sqlite3_file {
public:
	SQLiteFile();
	SQLiteFile(sqlite3_file *original_file);

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
 * By default, it forwards every call to the `original_vfs` passed to it in its constructor.
 *
 * @see https://sqlite.org/c3ref/vfs.html
 */
class SQLiteVfs : sqlite3_vfs {
public:
	SQLiteVfs(const char *name, const char *base_vfs_name = nullptr);
	SQLiteVfs(const char *name, int file_shim_size, const char *base_vfs_name = nullptr);
	SQLiteVfs(const char *name, int file_shim_size, sqlite3_vfs *original_vfs);
	
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

#endif  // __SQLITE_VFS_HPP__

///////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////
#ifdef SQLITE_VFS_SHIM_IMPLEMENTATION

template<auto SQLiteFile::*fptr, typename... Args>
static auto wrap_file_method(sqlite3_file *file, Args... args) {
	return (((SQLiteFile *) file)->*fptr)(args...);
}

static sqlite3_io_methods SQLiteFileShimMethods = {
	3,
	wrap_file_method<&SQLiteFile::xClose>,
	wrap_file_method<&SQLiteFile::xRead>,
	wrap_file_method<&SQLiteFile::xWrite>,
	wrap_file_method<&SQLiteFile::xTruncate>,
	wrap_file_method<&SQLiteFile::xSync>,
	wrap_file_method<&SQLiteFile::xFileSize>,
	wrap_file_method<&SQLiteFile::xLock>,
	wrap_file_method<&SQLiteFile::xUnlock>,
	wrap_file_method<&SQLiteFile::xCheckReservedLock>,
	wrap_file_method<&SQLiteFile::xFileControl>,
	wrap_file_method<&SQLiteFile::xSectorSize>,
	wrap_file_method<&SQLiteFile::xDeviceCharacteristics>,
	wrap_file_method<&SQLiteFile::xShmMap>,
	wrap_file_method<&SQLiteFile::xShmLock>,
	wrap_file_method<&SQLiteFile::xShmBarrier>,
	wrap_file_method<&SQLiteFile::xShmUnmap>,
	wrap_file_method<&SQLiteFile::xFetch>,
	wrap_file_method<&SQLiteFile::xUnfetch>,
};

// File
SQLiteFile::SQLiteFile()
	: sqlite3_file()
{
}
SQLiteFile::SQLiteFile(sqlite3_file *original_file)
	: sqlite3_file({ &SQLiteFileShimMethods }), original_file(original_file)
{
}

int SQLiteFile::xClose() {
	return original_file->pMethods->xClose(original_file);
}
int SQLiteFile::xRead(void *p, int iAmt, sqlite3_int64 iOfst) {
	return original_file->pMethods->xRead(original_file, p, iAmt, iOfst);
}
int SQLiteFile::xWrite(const void *p, int iAmt, sqlite3_int64 iOfst) {
	return original_file->pMethods->xWrite(original_file, p, iAmt, iOfst);
}
int SQLiteFile::xTruncate(sqlite3_int64 size) {
	return original_file->pMethods->xTruncate(original_file, size);
}
int SQLiteFile::xSync(int flags) {
	return original_file->pMethods->xSync(original_file, flags);
}
int SQLiteFile::xFileSize(sqlite3_int64 *pSize) {
	return original_file->pMethods->xFileSize(original_file, pSize);
}
int SQLiteFile::xLock(int flags) {
	return original_file->pMethods->xLock(original_file, flags);
}
int SQLiteFile::xUnlock(int flags) {
	return original_file->pMethods->xUnlock(original_file, flags);
}
int SQLiteFile::xCheckReservedLock(int *pResOut) {
	return original_file->pMethods->xCheckReservedLock(original_file, pResOut);
}
int SQLiteFile::xFileControl(int op, void *pArg) {
	return original_file->pMethods->xFileControl(original_file, op, pArg);
}
int SQLiteFile::xSectorSize() {
	return original_file->pMethods->xSectorSize(original_file);
}
int SQLiteFile::xDeviceCharacteristics() {
	return original_file->pMethods->xDeviceCharacteristics(original_file);
}
int SQLiteFile::xShmMap(int iPg, int pgsz, int flags, void volatile **pp) {
	return original_file->pMethods->xShmMap(original_file, iPg, pgsz, flags, pp);
}
int SQLiteFile::xShmLock(int offset, int n, int flags) {
	return original_file->pMethods->xShmLock(original_file, offset, n, flags);
}
void SQLiteFile::xShmBarrier() {
	return original_file->pMethods->xShmBarrier(original_file);
}
int SQLiteFile::xShmUnmap(int deleteFlag) {
	return original_file->pMethods->xShmUnmap(original_file, deleteFlag);
}
int SQLiteFile::xFetch(sqlite3_int64 iOfst, int iAmt, void **pp) {
	return original_file->pMethods->xFetch(original_file, iOfst, iAmt, pp);
}
int SQLiteFile::xUnfetch(sqlite3_int64 iOfst, void *p) {
	return original_file->pMethods->xUnfetch(original_file, iOfst, p);
}

// VFS
template<auto SQLiteVfs::*fptr, typename... Args>
static auto wrap_vfs_method(sqlite3_vfs *vfs, Args... args) {
	return (((SQLiteVfs *) vfs)->*fptr)(args...);
}

SQLiteVfs::SQLiteVfs(const char *name, const char *base_vfs_name)
	: SQLiteVfs(name, sizeof(SQLiteFile), base_vfs_name)
{
}
SQLiteVfs::SQLiteVfs(const char *name, int file_shim_size, const char *base_vfs_name)
	: SQLiteVfs(name, file_shim_size, sqlite3_vfs_find(base_vfs_name) ?: sqlite3_vfs_find(nullptr))
{
}
SQLiteVfs::SQLiteVfs(const char *name, int file_shim_size, sqlite3_vfs *original_vfs)
	: original_vfs(original_vfs)
{
	iVersion = original_vfs->iVersion;
	szOsFile = (int) file_shim_size + original_vfs->szOsFile;
	mxPathname = original_vfs->mxPathname;
	pNext = nullptr;
	zName = name;
	pAppData = nullptr;
	sqlite3_vfs::xOpen = wrap_vfs_method<&SQLiteVfs::xOpen>;
	sqlite3_vfs::xDelete = wrap_vfs_method<&SQLiteVfs::xDelete>;
	sqlite3_vfs::xAccess = wrap_vfs_method<&SQLiteVfs::xAccess>;
	sqlite3_vfs::xFullPathname = wrap_vfs_method<&SQLiteVfs::xFullPathname>;
	sqlite3_vfs::xDlOpen = wrap_vfs_method<&SQLiteVfs::xDlOpen>;
	sqlite3_vfs::xDlError = wrap_vfs_method<&SQLiteVfs::xDlError>;
	sqlite3_vfs::xDlSym = wrap_vfs_method<&SQLiteVfs::xDlSym>;
	sqlite3_vfs::xDlClose = wrap_vfs_method<&SQLiteVfs::xDlClose>;
	sqlite3_vfs::xRandomness = wrap_vfs_method<&SQLiteVfs::xRandomness>;
	sqlite3_vfs::xSleep = wrap_vfs_method<&SQLiteVfs::xSleep>;
	sqlite3_vfs::xCurrentTime = wrap_vfs_method<&SQLiteVfs::xCurrentTime>;
	sqlite3_vfs::xGetLastError = wrap_vfs_method<&SQLiteVfs::xGetLastError>;
	sqlite3_vfs::xCurrentTimeInt64 = wrap_vfs_method<&SQLiteVfs::xCurrentTimeInt64>;
	sqlite3_vfs::xSetSystemCall = wrap_vfs_method<&SQLiteVfs::xSetSystemCall>;
	sqlite3_vfs::xGetSystemCall = wrap_vfs_method<&SQLiteVfs::xGetSystemCall>;
	sqlite3_vfs::xNextSystemCall = wrap_vfs_method<&SQLiteVfs::xNextSystemCall>;
}

int SQLiteVfs::register_vfs(bool makeDefault) {
	return sqlite3_vfs_register(this, makeDefault);
}

int SQLiteVfs::unregister_vfs() {
	return sqlite3_vfs_unregister(this);
}

int SQLiteVfs::xOpen(sqlite3_filename zName, sqlite3_file *file, int flags, int *pOutFlags) {
	return open<SQLiteFile>(zName, file, flags, pOutFlags);
}
int SQLiteVfs::xDelete(const char *zName, int syncDir) {
	return original_vfs->xDelete(original_vfs, zName, syncDir);
}
int SQLiteVfs::xAccess(const char *zName, int flags, int *pResOut) {
	return original_vfs->xAccess(original_vfs, zName, flags, pResOut);
}
int SQLiteVfs::xFullPathname(const char *zName, int nOut, char *zOut) {
	return original_vfs->xFullPathname(original_vfs, zName, nOut, zOut);
}
void *SQLiteVfs::xDlOpen(const char *zFilename) {
	return original_vfs->xDlOpen(original_vfs, zFilename);
}
void SQLiteVfs::xDlError(int nByte, char *zErrMsg) {
	original_vfs->xDlError(original_vfs, nByte, zErrMsg);
}
void (*SQLiteVfs::xDlSym(void *library, const char *zSymbol))(void) {
	return original_vfs->xDlSym(original_vfs, library, zSymbol);
}
void SQLiteVfs::xDlClose(void *library) {
	return original_vfs->xDlClose(original_vfs, library);
}
int SQLiteVfs::xRandomness(int nByte, char *zOut) {
	return original_vfs->xRandomness(original_vfs, nByte, zOut);
}
int SQLiteVfs::xSleep(int microseconds) {
	return original_vfs->xSleep(original_vfs, microseconds);
}
int SQLiteVfs::xCurrentTime(double *pResOut) {
	return original_vfs->xCurrentTime(original_vfs, pResOut);
}
int SQLiteVfs::xGetLastError(int nByte, char *zOut) {
	return original_vfs->xGetLastError(original_vfs, nByte, zOut);
}
int SQLiteVfs::xCurrentTimeInt64(sqlite3_int64 *pResOut) {
	return original_vfs->xCurrentTimeInt64(original_vfs, pResOut);
}
int SQLiteVfs::xSetSystemCall(const char *zName, sqlite3_syscall_ptr ptr) {
	return original_vfs->xSetSystemCall(original_vfs, zName, ptr);
}
sqlite3_syscall_ptr SQLiteVfs::xGetSystemCall(const char *zName) {
	return original_vfs->xGetSystemCall(original_vfs, zName);
}
const char *SQLiteVfs::xNextSystemCall(const char *zName) {
	return original_vfs->xNextSystemCall(original_vfs, zName);
}

#endif