#include "src/meinekraft.hpp"

#ifdef WIN32
int wmain() {
#else
int main() {
#endif
  MeineKraft::instance().init();
  MeineKraft::instance().mainloop();
  return 0;
}
