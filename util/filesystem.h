#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <cstdlib>
#include <wchar.h>

class FileSystem
{
private:
  typedef std::string (*Builder) (const std::string& path);

public:
  static std::string getPath(const char * logl_root,const std::string& path)
  {
    //static std::string(*pathBuilder)(std::string const &) = getPathBuilder(path,logl_root);
   // return (*pathBuilder)(path);
	  return getPathRelativeRoot(logl_root, path);
  }

private:
  static std::string const & getRoot(const char * logl_root)
  {
    static char const * envRoot = getenv("LOGL_ROOT_PATH");
    static char const * givenRoot = (envRoot != nullptr ? envRoot : logl_root);
    static std::string root = (givenRoot != nullptr ? givenRoot : "");
    return root;
  }

  //static std::string(*foo (std::string const &)) getPathBuilder()
  static Builder getPathBuilder(const std::string& path,const char * logl_root)
  {
	  return &FileSystem::getPathRelativeBinary;
  }

  static std::string getPathRelativeRoot(const char * logl_root, const std::string& path)
  {
    return logl_root + path;
  }

  static std::string getPathRelativeBinary(const std::string& path)
  {
    return "../../../" + path;
  }
};

// FILESYSTEM_H
#endif
