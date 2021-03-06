/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "web/FileUtils.h"

#ifndef WT_HAVE_POSIX_FILEIO
#include <boost/filesystem/operations.hpp>
#else //WT_HAVE_POSIX_FILEIO
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdexcept>
#endif //WT_HAVE_POSIX_FILEIO

#include "web/WebUtils.h"
#include "Wt/WLogger"

#ifdef WIN32
#include <windows.h>
#endif // WIN32

namespace Wt {
  LOGGER("FileUtils");

  namespace FileUtils {
    unsigned long long size(const std::string &file) 
    {
#ifndef WT_HAVE_POSIX_FILEIO
      return (unsigned long long) boost::filesystem::file_size(file);
#else //WT_HAVE_POSIX_FILEIO
      struct stat sb;
      if (stat(file.c_str(), &sb) == -1) {
	std::string error 
	  = "size: stat failed for file \"" + file + "\"";
	LOG_ERROR(error);
	throw std::runtime_error(error);
      }
      return (unsigned long long)sb.st_size;
#endif //WT_HAVE_POSIX_FILEIO
    }

    time_t lastWriteTime(const std::string &file) 
    {
#ifndef WT_HAVE_POSIX_FILEIO
      return (unsigned long long)boost::filesystem::last_write_time(file);
#else //WT_HAVE_POSIX_FILEIO
      struct stat sb;
      if (stat(file.c_str(), &sb) == -1) {
	std::string error 
	  = "lastWriteTime: stat failed for file \"" + file + "\"";
	LOG_ERROR(error);
	throw std::runtime_error(error);
      }
      return (unsigned long long)sb.st_mtime;
#endif //WT_HAVE_POSIX_FILEIO
    }

    bool exists(const std::string &file) 
    {
#ifndef WT_HAVE_POSIX_FILEIO
      boost::filesystem::path path(file);
      return boost::filesystem::exists(path);
#else //WT_HAVE_POSIX_FILEIO
      struct stat sb;
      return stat(file.c_str(), &sb) != -1;
#endif //WT_HAVE_POSIX_FILEIO
    }

    bool isDirectory(const std::string &file) 
    {
#ifndef WT_HAVE_POSIX_FILEIO
      boost::filesystem::path path(file);
      return boost::filesystem::is_directory(path);
#else //WT_HAVE_POSIX_FILEIO
      struct stat sb;
      stat(file.c_str(), &sb);
      if (stat(file.c_str(), &sb) == -1) {
	std::string error 
	  = "isDirectory: stat failed for file \"" + file + "\"";
	LOG_ERROR(error);
	throw std::runtime_error(error);
      }
      return S_ISDIR(sb.st_mode);
#endif //WT_HAVE_POSIX_FILEIO
    }

    void listFiles(const std::string &directory, 
		   std::vector<std::string> &files) 
    {
#ifndef WT_HAVE_POSIX_FILEIO
      boost::filesystem::path path(directory);
      boost::filesystem::directory_iterator end_itr;

      if (!boost::filesystem::is_directory(path)) {
	std::string error 
	  = "listFiles: \"" + directory + "\" is not a directory";
	LOG_ERROR(error);
	throw std::runtime_error(error);
      }
      
      for (boost::filesystem::directory_iterator i(path); i != end_itr; ++i) {
#if BOOST_FILESYSTEM_VERSION < 3
	std::string f = Utils::lowerCase((*i).path().leaf());
#else //BOOST_FILESYSTEM_VERSION < 3
	std::string f = Utils::lowerCase((*i).path().leaf().string());
#endif //BOOST_FILESYSTEM_VERSION < 3
	files.push_back(f);
      }
#else //WT_HAVE_POSIX_FILEIO
      DIR *dp;
      struct dirent *dirp;
      if((dp = opendir(directory.c_str())) == NULL) {
	std::string error 
	   = "listFiles: opendir failed for file \"" + directory + "\"";
	LOG_ERROR(error);
	throw std::runtime_error(error);
      }
      
      while ((dirp = readdir(dp)) != NULL)
        files.push_back(dirp->d_name);

      closedir(dp);
#endif //WT_HAVE_POSIX_FILEIO
    }

    std::string getTempDir()
    {
      std::string tempDir;
      
      char *wtTmpDir = getenv("WT_TMP_DIR");
      if (wtTmpDir)
      tempDir = wtTmpDir;
      else {
#ifdef WIN32
	char winTmpDir[MAX_PATH];
	if(GetTempPathA(sizeof(winTmpDir), winTmpDir) != 0)
	  tempDir = winTmpDir;
#else
	tempDir = "/tmp";
#endif
      }
      
      return tempDir;
    }
    
    extern std::string createTempFileName()
    {
      std::string tempDir = getTempDir();
      
#ifdef WIN32
      char tmpName[MAX_PATH];
      
      if(tempDir == "" 
	 || GetTempFileNameA(tempDir.c_str(), "wt-", 0, tmpName) == 0)
	return "";
      
      return tmpName;
#else
      char* spool = new char[20 + tempDir.size()];
      strcpy(spool, (tempDir + "/wtXXXXXX").c_str());
      
      int i = mkstemp(spool);
      close(i);
      
      std::string returnSpool = spool;
      delete [] spool;
      return returnSpool;
#endif
    }

  }
}
