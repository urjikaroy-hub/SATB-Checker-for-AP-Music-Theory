#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "usage: satb file.satb\n";
    return 2;
  }
  Score sc;
  string err;
  if (!= parseFile(argv[1], sc, err)) {
    cerr << "error: " << err << "\n";
    return 2;
  }
  for (size_t i = 0; i < sc.chords.size(); i++) {
    if (sc.chords[i].skip)
      cerr << "warning: chord " << i + 1 << " '" << sc.chords[i].label
           << "' harmony not checked \n";
    checkVertical(sc, i);
    if (i > 0)
      checkHorizontal(sc, i);
  }
  for (const auto &x : found)
    printf("chord %zu (%s) %-15s %s\n", x.col + 1,
           sc.chords[x.col].label.c_str(), x.rule.c_str, x.detail.c_str());
  printf("%zu issue(s) \n", found.size());
  return found.empty() ? 0 : 1;
}