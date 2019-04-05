#pragma once
#ifndef MEINEKRAFT_FILESYSTEM_HPP
#define MEINEKRAFT_FILESYSTEM_HPP

// TODO: Make these dynamic rather than static
namespace Filesystem {
  /// Base filepath to the executeable
#if defined(__linux__)
  const std::string base = "/home/alexander/repos/MeineKraft/";
#elif defined(__APPLE__)
  const std::string base = "/Users/lingtorp/repos/MeineKraft/";
#elif defined(_WIN32) || defined(_WIN64)
  const std::string base = "C:/Users/Alexander/repos/MeineKraft/";
#endif
  
#if defined(__linux__)
  const std::string home = "/home/alexander/";
#elif defined(__APPLE__)
  const std::string home = "/Users/lingtorp/";
#elif defined(_WIN32) || defined(_WIN64)
  const std::string home = "C:/Users/Alexander/";
#endif
}

#endif // MEINEKRAFT_FILESYSTEM_HPP