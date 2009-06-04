/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_io_File_H
#define db_io_File_H

#include "db/io/ByteBuffer.h"
#include "db/rt/Exception.h"
#include "db/rt/WindowsSupport.h"
#include "db/util/Date.h"

#include <string>

namespace db
{
namespace io
{

#ifdef WIN32
#   ifdef BUILD_DB_IO_DLL
#      define DLL_CLASS __WIN32_DLL_EXPORT
#   else
#      define DLL_CLASS __WIN32_DLL_IMPORT
#   endif
#else
#   define DLL_CLASS
#endif

// forward declare file, file list
class File;
class FileList;

/**
 * The FileImpl class provides the implementation for the reference-counted
 * File class.
 * 
 * FIXME: this class needs to be filled out with simple file meta-data
 * retrieving methods
 * 
 * @author Dave Longley
 * @author Manu Sporny
 */
class FileImpl
{
public:
   /**
    * The types of files.
    */
   enum Type
   {
      RegularFile, Directory, SymbolicLink, Socket, Unknown
   };
   
protected:
   /**
    * Stores the path of this file, as set by the user.
    */
   char* mPath;
   
   /**
    * Stores the absolute path of this file.
    */
   char* mAbsolutePath;
   
   /**
    * Stores the base name of this file.
    */
   char* mBaseName;
   
   /**
    * Stores the canonical path of this file.
    */
   char* mCanonicalPath;
   
   /**
    * Stores the extension of this file.
    */
   char* mExtension;
   
   /**
    * Set to true to remove a file when cleaning up.
    */
   bool mRemoveOnCleanup;
   
public:
   /**
    * Creates a new File.
    */
   FileImpl();
   
   /**
    * Creates a new File with the specified path. If the OS is MS windows, then
    * forward slash directory separators will be converted to back slashes.
    * 
    * @param path the path of the file. Using a blank string specifies an 
    *             invalid file and should never be done. If you want to mention
    *             the current directory, use "." as the path.
    */
   FileImpl(const char* path);
   
   /**
    * Destructs this File.
    */
   virtual ~FileImpl();
   
   /**
    * Creates a new file, overwriting an existing one of the same name.
    * 
    * @return true if the file was created, false if not.
    */
   virtual bool create();
   
   /**
    * Creates all directories in this file's path that do not already exist.
    * If an exception occurs, then some of the directories, but not all, may
    * have been created.
    * 
    * @return true if all directories were created, false if an exception
    *         occurred.
    */
   virtual bool mkdirs();
   
   /**
    * Determines whether or not this file physically exists.
    * 
    * @return true if this file exists, false if not.
    */
   virtual bool exists();
   
   /**
    * Deletes this file, if it exists.
    * 
    * @return true if this file was deleted, false if not.
    */
   virtual bool remove();
   
   /**
    * Sets whether or not this file should be deleted when this object
    * is destructed.
    * 
    * @param remove true to remove when destructing, false not to.
    */
   virtual void setRemoveOnCleanup(bool remove);
   
   /**
    * Renames this file to the passed one, overwriting an existing file of the
    * same name.
    * 
    * @param file the File to rename this one to.
    * 
    * @return true if the rename was successful, false if not.
    */
   virtual bool rename(File& file);
   
   /**
    * Gets the name of this File. This is just the last name in the
    * path for the File.
    * 
    * @return the name of this File.
    */
   virtual const char* getBaseName();
   
   /**
    * Gets the path to this File. This is not necessarily an absolute
    * or normalized path.
    * 
    * @return the path to this File.
    */
   virtual const char* getPath() const;
   
   /**
    * Gets the absolute and unique path to this File. The path will be
    * normalized and, if necessary, have the current working directory
    * prepended to it to resolve a relative path. Symbolic links will
    * not be followed.
    * 
    * @return the absolute path to this File.
    */
   virtual const char* getAbsolutePath() const;
   
   /**
    * Gets the absolute path to this File, resolving any symbolic links
    * encountered.
    * 
    * @return the canonical path to this File.
    */
   virtual const char* getCanonicalPath();
   
   /**
    * Gets the extension of this File. This is the part of the path name
    * following the last '.'.
    * 
    * @return the extension of this File.
    */
   virtual const char* getExtension();
   
   /**
    * Gets the length of this File.
    * 
    * @return the length of this File.
    */
   virtual off_t getLength();
   
   /**
    * Gets the Type of File.
    * 
    * @param follow true to follow symbolic links, false not to.
    * 
    * @return the Type of File this File is.
    */
   virtual Type getType(bool follow = true);
   
   /**
    * Returns true if this File contains the given file path. Both
    * file paths are fully normalized before the comparison is made. All ".."s
    * are removed, drive letters are applied (if applicable), and superfluous
    * directory/file separators are cleaned from the file path. 
    * 
    * @param path the path to check against the current file.
    * @return true if this File is an ancestor directory to the given path,
    *         false otherwise.
    */
   virtual bool contains(const char* path);

   /**
    * Returns true if this File contains the given file path. Both
    * file paths are fully normalized before the comparison is made. All ".."s
    * are removed, drive letters are applied (if applicable), and superfluous
    * directory/file separators are cleaned from the file path. 
    * 
    * @param path the path to check against the current file.
    * @return true if this File is an ancestor directory to the given path,
    *         false otherwise.
    */
   virtual bool contains(File& path);
   
   /**
    * Returns true if this File is a directory, false if it is not. If it
    * is not, then it may be a regular file or a symbolic link. 
    * 
    * @return true if this File is a directory, false if not.
    */
   virtual bool isDirectory();
   
   /**
    * Returns true if this File is a regular file, false if it is not. If it
    * is not, then it may be a directory or a symbolic link. 
    * 
    * @return true if this File is a regular file, false if not.
    */
   virtual bool isFile();

   /**
    * Returns true if this File is a root path, false if not.
    * 
    * On unix-based systems, this will check the absolute path to see if it
    * is "/". On MS windows systems, this well check the absolute path to
    * see if it is a drive letter, i.e. "C:\" or "d:\".
    * 
    * @return true if this File is a root path, false if not.
    */
   virtual bool isRoot();
   
   /**
    * Returns true if this File is readable, false otherwise. Readability
    * depends on several things, including file permissions, file system
    * permissions, and access control lists among other file security 
    * mechanisms. 
    * 
    * @return true if this File is readable, false if not.
    */
   virtual bool isReadable();
   
   /**
    * Returns true if this File is a symbolic link, false if it is not. If it
    * is not, then it may be a regular file or a directory. 
    * 
    * @return true if this File is a symbolic link, false if not.
    */
   virtual bool isSymbolicLink();

   /**
    * Returns true if this File is writable, false otherwise. Readability
    * depends on several things, including file permissions, file system
    * permissions, and access control lists among other file security 
    * mechanisms. 
    * 
    * @return true if this File is writable, false if not.
    */
   virtual bool isWritable();
   
   /**
    * Populates a list with all of the Files in this File, if this File is
    * a directory. Each File added to the list will be heap-allocated, and it
    * is assumed that the passed list will manage their memory.
    * 
    * @param files the FileList to populate.
    */
   virtual void listFiles(FileList& files);
   
   /**
    * Get a date object representing the last modified time of this File.
    * 
    * @return date this file was last modified or Date(0) on error.
    */
   virtual db::util::Date getModifiedDate();
};

/**
 * The reference-counted File class. A File represents a file or directory
 * on a disk.
 * 
 * @author Dave Longley
 */
class DLL_CLASS File : public db::rt::Collectable<FileImpl>
{
public:
   /**
    * The OS-dependent separator between names in a file path,
    * i.e. "/" for "/foo/bar".
    */
   static const char* NAME_SEPARATOR;
   static const char NAME_SEPARATOR_CHAR;
   
   /**
    * The OS-dependent separator between entires paths,
    * i.e. ":" for "/foo/bar:/foo/moo".
    */
   static const char* PATH_SEPARATOR;
   static const char PATH_SEPARATOR_CHAR;
   
public:
   /**
    * Creates a new File object.
    * 
    * @param path an optional path name.
    */
   File(const char* path) :
      db::rt::Collectable<FileImpl>(new FileImpl(path)) {};
   
   /**
    * Creates a File object.
    * 
    * @param impl the FileImpl to reference.
    */
   File(FileImpl* impl) :
      db::rt::Collectable<FileImpl>(impl) {};
   
   /**
    * Destructs this File.
    */
   virtual ~File() {};
   
   /**
    * Returns true if this File is equal to the passed one. Two Files are
    * only equal if their names are the same and they are the same type,
    * meaning they are both regular files, both directories, or both
    * symbolic links.
    * 
    * @param rhs the File to compare to this one.
    * 
    * @return true if this File is equal to the passed one, false if not.
    */
   bool operator==(const File& rhs) const;
   
   /**
    * Returns true if this File is NOT equal to the passed one. Returns
    * the opposite of operator==.
    * 
    * @param rhs the File to compare to this one.
    * 
    * @return true if this File is NOT equal to the passed one, false if not.
    */
   bool operator!=(const File& rhs) const;
   
   /**
    * Reads the entire contents of this file into the passed ByteBuffer.
    * Useful for small files only.
    * 
    * @param buffer the buffer to populate, it will not be resized.
    * 
    * @return true if successful, false if an exception occurred.
    */
   bool readBytes(ByteBuffer* buffer);
   
   /**
    * Writes the entire contents of the passed ByteBuffer into this file,
    * without modifying the ByteBuffer.
    * 
    * @param buffer the buffer with data to write.
    * @param append true to append to the file, if it exists,
    *               false to overwrite.
    * 
    * @return true if successful, false if an exception occurred.
    */
   bool writeBytes(ByteBuffer* buffer, bool append);
   
   /**
    * Gets the absolute path for the given path. This method will normalize
    * the passed file system path by resolving all "." and ".." directories and
    * prepending the current working directory if necessary to a relative path.
    * 
    * @param path the path to get the absolute path for.
    * @param absolutePath to store the absolute path.
    * 
    * @return true if successful, false if an Exception occurred.
    */
   static bool getAbsolutePath(const char* path, std::string& absolutePath);
   
   /**
    * Gets the canonical path for the given path. This method will get the
    * absolute path for the given path and follow any symbolic links.
    * 
    * @param path the path to get the canonical path for.
    * @param canonicalPath to store the canonical path.
    * 
    * @return true if successful, false if an Exception occurred.
    */
   static bool getCanonicalPath(const char* path, std::string& canonicalPath);
   
   /**
    * Normalizes the file system path passed into the method. This method will
    * resolve all "." and ".." directories in the file's path.
    * 
    * @param path the path to normalize as a regular constant string.
    * @param normalizedPath the normalized path will be placed into this 
    *                       variable.
    * 
    * @return true if the normalization was successful, false if an Exception
    *         occurred.
    */
   static bool normalizePath(const char* path, std::string& normalizedPath);
   
   /**
    * Expand "~" at the beginning of a path into the current user.  Will fail
    * if "HOME" is not set in the environment.
    * NOTE: Currently only handles current user.
    * 
    * @param path the path to be expanded, if it contains a '~' it will be
    *             expanded to the current user's home path.
    * @param expandedPath to store the expanded path.
    * 
    * @return true if the expansion was successful, false if an Exception
    *         occurred.
    */
   static bool expandUser(const char* path, std::string& expandedPath);
   
   /**
    * Gets the current working directory. 
    *
    * @param cwd the string that will be updated with the current working
    *            directory upon success.
    * 
    * @return false if the current working directory could not be
    *         retrieved (with an Exception set), true upon success.
    */
   static bool getCurrentWorkingDirectory(std::string& cwd);
   
   /**
    * Gets the system temporary directory. 
    * 
    * @param tmp the string that will be updated with the system's temporary
    *            directory upon success.
    * 
    * @return false if the temporary directory could not be
    *         retrieved (with an Exception set), true upon success.
    */
   static bool getTemporaryDirectory(std::string& tmp);
   
   /**
    * Creates a temporary file. This file will be deleted when it goes out
    * of scope. It may be manually deleted via remove() if desired.
    * 
    * @param prefix a prefix for the temporary file.
    * @param dir the directory for the file, NULL for the default system
    *            temporary directory.
    * 
    * @return the temporary file, or NULL if no file could be created.
    */
   static File createTempFile(const char* prefix, const char* dir = NULL);
   
   /**
    * Determines if the passed path is readable or not.
    * 
    * @param path the path to check for readability.
    * 
    * @return true if the passed path is readable, false if not.
    */
   static bool isPathReadable(const char* path);
   
   /**
    * Determines if the passed path is writable or not.
    * 
    * @param path the path to check for writability.
    * 
    * @return true if the passed path is writable, false if not.
    */
   static bool isPathWritable(const char* path);

   /**
    * Splits a path into the head (dirname) and tail (basename).
    * 
    * dirname is everything up to the final path separator. If path ends in
    * a path separator basename will be empty. If there is no path separator
    * in the path dirname will be empty. Any trailing separator are stripped
    * from dirname unless it is the root.
    * 
    * @param path the path to split.
    * @param dirname a string to fill with the dirname.
    * @param basename a string to fill with the basename.
    */
   static void split(
      const char* path, std::string& dirname, std::string& basename);
   
   /**
    * Splits a path into the root and extension, either of which may be empty.
    * 
    * @param path the path to split.
    * @param root a string to fill with the root.
    * @param ext a string to fill with the extension.
    * @param sep string separating root and extension 
    */
   static void splitext(
      const char* path, std::string& root, std::string& ext,
      const char* sep = ".");
   
   /**
    * Gets the parent directory of the passed path. Works for both files
    * and directories. If the passed path is "/", then "/" will be returned.
    * 
    * @param path the path to get the parent directory of.
    * 
    * @return the parentname of path.
    */
   static std::string parentname(const char* path);
   
   /**
    * Convienience to get dirname from split().
    * 
    * @param path the path to split.
    * 
    * @return the dirname of path.
    */
   static std::string dirname(const char* path);

   /**
    * Convienience to get basename from split().
    * 
    * @param path the path to split.
    * 
    * @return the basename of path.
    */
   static std::string basename(const char* path);

   /**
    * Check if a path is absolute or relative.
    * 
    * @param path the path to examine.
    * 
    * @return true if absolute, false if relative.
    */
   static bool isPathAbsolute(const char* path);
   
   /**
    * Returns true if a path is a root path, false if not.
    * 
    * On unix-based systems, this will check the absolute path to see if it
    * is "/". On MS windows systems, this well check the absolute path to
    * see if it is a drive letter, i.e. "C:\" or "d:\".
    * 
    * @return true if a root path, false if not.
    */
   static bool isPathRoot(const char* path);
   
   /**
    * Join two paths with a name separator.  A trailing separator on the first
    * path or a leading separator on the second path will first be removed to
    * result in only a single name separator.
    * 
    * @param path1 first path.
    * @param path2 second path.
    * 
    * @return a string with the joined paths.
    */
   static std::string join(const char* path1, const char* path2);
};

#undef DLL_CLASS

} // end namespace io
} // end namespace db
#endif
