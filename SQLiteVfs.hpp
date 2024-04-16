#ifndef __SQLITE_VFS_HPP__
#define __SQLITE_VFS_HPP__

#include <sqlite3.h>

namespace sqlite3vfs {
	/**
	 * SQLite File implementation with virtual methods for C++.
	 *
	 * By default, it forwards every call to the `original_file` passed to it by `SQLiteFile`.
	 * 
	 * @note Destructors will be called automatically by `SQLiteFile` right after `xClose` is called.
	 *
	 * @see https://sqlite.org/c3ref/file.html
	 */
	struct SQLiteFileImpl {
		sqlite3_file *original_file;

		virtual int iVersion() const {
			return original_file ? original_file->pMethods->iVersion : 1;
		}

		virtual int xClose() {
			return original_file->pMethods->xClose(original_file);
		}
		virtual int xRead(void *p, int iAmt, sqlite3_int64 iOfst) {
			return original_file->pMethods->xRead(original_file, p, iAmt, iOfst);
		}
		virtual int xWrite(const void *p, int iAmt, sqlite3_int64 iOfst) {
			return original_file->pMethods->xWrite(original_file, p, iAmt, iOfst);
		}
		virtual int xTruncate(sqlite3_int64 size) {
			return original_file->pMethods->xTruncate(original_file, size);
		}
		virtual int xSync(int flags) {
			return original_file->pMethods->xSync(original_file, flags);
		}
		virtual int xFileSize(sqlite3_int64 *pSize) {
			return original_file->pMethods->xFileSize(original_file, pSize);
		}
		virtual int xLock(int flags) {
			return original_file->pMethods->xLock(original_file, flags);
		}
		virtual int xUnlock(int flags) {
			return original_file->pMethods->xUnlock(original_file, flags);
		}
		virtual int xCheckReservedLock(int *pResOut) {
			return original_file->pMethods->xCheckReservedLock(original_file, pResOut);
		}
		virtual int xFileControl(int op, void *pArg) {
			return original_file->pMethods->xFileControl(original_file, op, pArg);
		}
		virtual int xSectorSize() {
			return original_file->pMethods->xSectorSize(original_file);
		}
		virtual int xDeviceCharacteristics() {
			return original_file->pMethods->xDeviceCharacteristics(original_file);
		}
		/* Methods above are valid for version 1 */
		virtual int xShmMap(int iPg, int pgsz, int flags, void volatile**pp) {
			return original_file->pMethods->xShmMap(original_file, iPg, pgsz, flags, pp);
		}
		virtual int xShmLock(int offset, int n, int flags) {
			return original_file->pMethods->xShmLock(original_file, offset, n, flags);
		}
		virtual void xShmBarrier() {
			return original_file->pMethods->xShmBarrier(original_file);
		}
		virtual int xShmUnmap(int deleteFlag) {
			return original_file->pMethods->xShmUnmap(original_file, deleteFlag);
		}
		/* Methods above are valid for version 2 */
		virtual int xFetch(sqlite3_int64 iOfst, int iAmt, void **pp) {
			return original_file->pMethods->xFetch(original_file, iOfst, iAmt, pp);
		}
		virtual int xUnfetch(sqlite3_int64 iOfst, void *p) {
			return original_file->pMethods->xUnfetch(original_file, iOfst, p);
		}
		/* Methods above are valid for version 3 */
		/* Additional methods may be added in future releases */
	};

	/**
	 * POD `sqlite3_file` subclass that forwards all invocations to an embedded object that inherits `SQLiteFileImpl`.
	 *
	 * You should not create objects of this type manually nor subclass it.
	 * Pass your `SQLiteFileImpl` subclass as template argument instead.
	 */
	template<typename TFileImpl>
	struct SQLiteFile : public sqlite3_file {
		using FileImpl = TFileImpl;

		sqlite3_io_methods methods;
		FileImpl implementation;
		sqlite3_file original_file[0];

		void setup(int open_result) {
			if (open_result == SQLITE_OK) {
				new (&implementation) FileImpl();
				implementation.original_file = original_file;
				methods = {
					implementation.iVersion(),
					&wrap_xClose,
					&wrap_xRead,
					&wrap_xWrite,
					&wrap_xTruncate,
					&wrap_xSync,
					&wrap_xFileSize,
					&wrap_xLock,
					&wrap_xUnlock,
					&wrap_xCheckReservedLock,
					&wrap_xFileControl,
					&wrap_xSectorSize,
					&wrap_xDeviceCharacteristics,
					&wrap_xShmMap,
					&wrap_xShmLock,
					&wrap_xShmBarrier,
					&wrap_xShmUnmap,
					&wrap_xFetch,
					&wrap_xUnfetch,
				};
				pMethods = &methods;
			}
			else {
				pMethods = nullptr;
			}
		}
	
	private:
		static int wrap_xClose(sqlite3_file *file) {
			int result = static_cast<SQLiteFile *>(file)->implementation.xClose();
			static_cast<SQLiteFile *>(file)->~SQLiteFile();
			return result;
		}
		static int wrap_xRead(sqlite3_file *file, void *p, int iAmt, sqlite3_int64 iOfst) {
			return static_cast<SQLiteFile *>(file)->implementation.xRead(p, iAmt, iOfst);
		}
		static int wrap_xWrite(sqlite3_file *file, const void *p, int iAmt, sqlite3_int64 iOfst) {
			return static_cast<SQLiteFile *>(file)->implementation.xWrite(p, iAmt, iOfst);
		}
		static int wrap_xTruncate(sqlite3_file *file, sqlite3_int64 size) {
			return static_cast<SQLiteFile *>(file)->implementation.xTruncate(size);
		}
		static int wrap_xSync(sqlite3_file *file, int flags) {
			return static_cast<SQLiteFile *>(file)->implementation.xSync(flags);
		}
		static int wrap_xFileSize(sqlite3_file *file, sqlite3_int64 *pSize) {
			return static_cast<SQLiteFile *>(file)->implementation.xFileSize(pSize);
		}
		static int wrap_xLock(sqlite3_file *file, int flags) {
			return static_cast<SQLiteFile *>(file)->implementation.xLock(flags);
		}
		static int wrap_xUnlock(sqlite3_file *file, int flags) {
			return static_cast<SQLiteFile *>(file)->implementation.xUnlock(flags);
		}
		static int wrap_xCheckReservedLock(sqlite3_file *file, int *pResOut) {
			return static_cast<SQLiteFile *>(file)->implementation.xCheckReservedLock(pResOut);
		}
		static int wrap_xFileControl(sqlite3_file *file, int op, void *pArg) {
			return static_cast<SQLiteFile *>(file)->implementation.xFileControl(op, pArg);
		}
		static int wrap_xSectorSize(sqlite3_file *file) {
			return static_cast<SQLiteFile *>(file)->implementation.xSectorSize();
		}
		static int wrap_xDeviceCharacteristics(sqlite3_file *file) {
			return static_cast<SQLiteFile *>(file)->implementation.xDeviceCharacteristics();
		}
		static int wrap_xShmMap(sqlite3_file *file, int iPg, int pgsz, int flags, void volatile**pp) {
			return static_cast<SQLiteFile *>(file)->implementation.xShmMap(iPg, pgsz, flags, pp);
		}
		static int wrap_xShmLock(sqlite3_file *file, int offset, int n, int flags) {
			return static_cast<SQLiteFile *>(file)->implementation.xShmLock(offset, n, flags);
		}
		static void wrap_xShmBarrier(sqlite3_file *file) {
			return static_cast<SQLiteFile *>(file)->implementation.xShmBarrier();
		}
		static int wrap_xShmUnmap(sqlite3_file *file, int deleteFlag) {
			return static_cast<SQLiteFile *>(file)->implementation.xShmUnmap(deleteFlag);
		}
		static int wrap_xFetch(sqlite3_file *file, sqlite3_int64 iOfst, int iAmt, void **pp) {
			return static_cast<SQLiteFile *>(file)->implementation.xFetch(iOfst, iAmt, pp);
		}
		static int wrap_xUnfetch(sqlite3_file *file, sqlite3_int64 iOfst, void *p) {
			return static_cast<SQLiteFile *>(file)->implementation.xUnfetch(iOfst, p);
		}
	};

	/**
	 * SQLite VFS implementation with virtual methods for C++.
	 *
	 * By default, it forwards every call to the `original_vfs` passed to it.
	 *
	 * @see https://sqlite.org/c3ref/vfs.html
	 */
	template<typename TFileImpl>
	struct SQLiteVfsImpl {
		using FileImpl = TFileImpl;
		
		sqlite3_vfs *original_vfs;
		
		virtual int xOpen(sqlite3_filename zName, SQLiteFile<TFileImpl> *file, int flags, int *pOutFlags) {
			int result = original_vfs->xOpen(original_vfs, zName, file->original_file, flags, pOutFlags);
			file->setup(result);
			return result;
		}
		virtual int xDelete(const char *zName, int syncDir) {
			return original_vfs->xDelete(original_vfs, zName, syncDir);
		}
		virtual int xAccess(const char *zName, int flags, int *pResOut) {
			return original_vfs->xAccess(original_vfs, zName, flags, pResOut);
		}
		virtual int xFullPathname(const char *zName, int nOut, char *zOut) {
			return original_vfs->xFullPathname(original_vfs, zName, nOut, zOut);
		}
		virtual void *xDlOpen(const char *zFilename) {
			return original_vfs->xDlOpen(original_vfs, zFilename);
		}
		virtual void xDlError(int nByte, char *zErrMsg) {
			original_vfs->xDlError(original_vfs, nByte, zErrMsg);
		}
		virtual void (*xDlSym(void *library, const char *zSymbol))(void) {
			return original_vfs->xDlSym(original_vfs, library, zSymbol);
		}
		virtual void xDlClose(void *library) {
			return original_vfs->xDlClose(original_vfs, library);
		}
		virtual int xRandomness(int nByte, char *zOut) {
			return original_vfs->xRandomness(original_vfs, nByte, zOut);
		}
		virtual int xSleep(int microseconds) {
			return original_vfs->xSleep(original_vfs, microseconds);
		}
		virtual int xCurrentTime(double *pResOut) {
			return original_vfs->xCurrentTime(original_vfs, pResOut);
		}
		virtual int xGetLastError(int nByte, char *zOut) {
			return original_vfs->xGetLastError(original_vfs, nByte, zOut);
		}
		/*
		** The methods above are in version 1 of the sqlite_vfs object
		** definition.  Those that follow are added in version 2 or later
		*/
		virtual int xCurrentTimeInt64(sqlite3_int64 *pResOut) {
			return original_vfs->xCurrentTimeInt64(original_vfs, pResOut);
		}
		/*
		** The methods above are in versions 1 and 2 of the sqlite_vfs object.
		** Those below are for version 3 and greater.
		*/
		virtual int xSetSystemCall(const char *zName, sqlite3_syscall_ptr ptr) {
			return original_vfs->xSetSystemCall(original_vfs, zName, ptr);
		}
		virtual sqlite3_syscall_ptr xGetSystemCall(const char *zName) {
			return original_vfs->xGetSystemCall(original_vfs, zName);
		}
		virtual const char *xNextSystemCall(const char *zName) {
			return original_vfs->xNextSystemCall(original_vfs, zName);
		}
		/*
		** The methods above are in versions 1 through 3 of the sqlite_vfs object.
		** New fields may be appended in future versions.  The iVersion
		** value will increment whenever this happens.
		*/
	};

	/**
	 * POD `sqlite3_vfs` subclass that forwards all invocations to an embedded object that inherits `SQLiteVfsImpl`.
	 *
	 * You should not subclass this type.
	 * Pass your `SQLiteVfsImpl` subclass as template argument instead.
	 */
	template<typename TVfsImpl>
	struct SQLiteVfs : public sqlite3_vfs {
		using VfsImpl = TVfsImpl;
		using FileImpl = typename VfsImpl::FileImpl;

		VfsImpl implementation;

		SQLiteVfs(const char *name)
			: SQLiteVfs(name, (sqlite3_vfs *) nullptr)
		{	
		}

		SQLiteVfs(const char *name, const char *base_vfs_name)
			: SQLiteVfs(name, sqlite3_vfs_find(base_vfs_name))
		{
		}

		SQLiteVfs(const char *name, sqlite3_vfs *original_vfs)
			: SQLiteVfs()
		{
			if (original_vfs == nullptr) {
				original_vfs = sqlite3_vfs_find(nullptr);
			}
			implementation.original_vfs = original_vfs;

			iVersion = original_vfs->iVersion;
			szOsFile = (int) sizeof(SQLiteFile<FileImpl>) + original_vfs->szOsFile;
			mxPathname = original_vfs->mxPathname;
			zName = name;
		}

		int register_vfs(bool makeDefault) {
			return sqlite3_vfs_register(this, makeDefault);
		}
		
		int unregister_vfs() {
			return sqlite3_vfs_unregister(this);
		}

	private:
		SQLiteVfs()
			: implementation()
		{
			pNext = nullptr;
			pAppData = nullptr;
			xOpen = &wrap_xOpen;
			xDelete = &wrap_xDelete;
			xAccess = &wrap_xAccess;
			xFullPathname = &wrap_xFullPathname;
			xDlOpen = &wrap_xDlOpen;
			xDlError = &wrap_xDlError;
			xDlSym = &wrap_xDlSym;
			xDlClose = &wrap_xDlClose;
			xRandomness = &wrap_xRandomness;
			xSleep = &wrap_xSleep;
			xCurrentTime = &wrap_xCurrentTime;
			xGetLastError = &wrap_xGetLastError;
			xCurrentTimeInt64 = &wrap_xCurrentTimeInt64;
			xSetSystemCall = &wrap_xSetSystemCall;
			xGetSystemCall = &wrap_xGetSystemCall;
			xNextSystemCall = &wrap_xNextSystemCall;
		}
		
		static int wrap_xOpen(sqlite3_vfs *vfs, sqlite3_filename zName, sqlite3_file *file, int flags, int *pOutFlags) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xOpen(zName, (SQLiteFile<FileImpl> *) file, flags, pOutFlags);
		}
		static int wrap_xDelete(sqlite3_vfs *vfs, const char *zName, int syncDir) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xDelete(zName, syncDir);
		}
		static int wrap_xAccess(sqlite3_vfs *vfs, const char *zName, int flags, int *pResOut) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xAccess(zName, flags, pResOut);
		}
		static int wrap_xFullPathname(sqlite3_vfs *vfs, const char *zName, int nOut, char *zOut) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xFullPathname(zName, nOut, zOut);
		}
		static void *wrap_xDlOpen(sqlite3_vfs *vfs, const char *zFilename) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xDlOpen(zFilename);
		}
		static void wrap_xDlError(sqlite3_vfs *vfs, int nByte, char *zErrMsg) {
			static_cast<SQLiteVfs *>(vfs)->implementation.xDlError(nByte, zErrMsg);
		}
		static void (*wrap_xDlSym(sqlite3_vfs *vfs, void *library, const char *zSymbol))(void) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xDlSym(library, zSymbol);
		}
		static void wrap_xDlClose(sqlite3_vfs *vfs, void *library) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xDlClose(library);
		}
		static int wrap_xRandomness(sqlite3_vfs *vfs, int nByte, char *zOut) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xRandomness(nByte, zOut);
		}
		static int wrap_xSleep(sqlite3_vfs *vfs, int microseconds) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xSleep(microseconds);
		}
		static int wrap_xCurrentTime(sqlite3_vfs *vfs, double *pResOut) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xCurrentTime(pResOut);
		}
		static int wrap_xGetLastError(sqlite3_vfs *vfs, int nByte, char *zOut) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xGetLastError(nByte, zOut);
		}
		static int wrap_xCurrentTimeInt64(sqlite3_vfs *vfs, sqlite3_int64 *pResOut) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xCurrentTimeInt64(pResOut);
		}
		static int wrap_xSetSystemCall(sqlite3_vfs *vfs, const char *zName, sqlite3_syscall_ptr ptr) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xSetSystemCall(zName, ptr);
		}
		static sqlite3_syscall_ptr wrap_xGetSystemCall(sqlite3_vfs *vfs, const char *zName) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xGetSystemCall(zName);
		}
		static const char *wrap_xNextSystemCall(sqlite3_vfs *vfs, const char *zName) {
			return static_cast<SQLiteVfs *>(vfs)->implementation.xNextSystemCall(zName);
		}
	};
}

#endif  // __SQLITE_VFS_HPP__
