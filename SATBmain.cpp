#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// Pitch Math
const int kSemi[7] = {0, 2, 4, 5, 7, 9, 11};
const string kLetters = "CDEFGAB", kVoice = "SATB";
string vn(int v) { return string(1, kVoice[v]); }

struct Pitch {
  int step = 0, acc = 0, oct = 4;
  string name;
  int dia() const { return oct * 7 + step; }
  int midi() const { return 12 * (oct + 1) + kSemi[step] + acc; }
};

bool parsePitch(const string &s, Pitch &p) {
  if (s.empty() || kLetters.find(toupper(s[0])) == string::npos)
    return false;
  p.step = (int)kLetters.find(toupper(s[0]));
  p.acc = 0;
  p.name = s;
  size_t i = 1;
  for (; i < s.size() && (s[i] == '#' || s[i] == 'b'); i++)
    p.acc += (s[i] == '#') ? 1 : -1;
  if (i >= s.size() || !isdigit(s[i]))
    return false;
  p.oct = stoi(s.substr(i));
  return true;
}

pair<int, int> ivl(const Pitch &a, const Pitch &b) {
  return {b.dia() - a.dia(), b.midi() - a.midi()};
}

int perfectType(const Pitch &a, const Pitch &b) {
  auto [st, se] = ivl(a, b);
  st = abs(st);
  se = abs(se);
  if (st % 7 == 4 && se % 12 == 7) return 5;
  if (st % 7 == 0 && se % 12 == 0) return 8;
  return 0;
}

// Harmony checker
struct Key {
  int tonicStep = 0, tonicAcc = 0;
  bool minor = false;
};
struct Chord {
  string label;
  int degree = -1;
  bool has7 = false, skip = false;
};

Chord parseRoman(const string &s) {
  Chord c;
  c.label = s;
  if (s.find('/') != string::npos) {
    c.skip = true;
    return c;
  }
  string r;
  for (char ch : s) {
    char u = toupper(ch);
    if (u != 'I' && u != 'V') break;
    r += u;
  }
  const string names[7] = {"I", "II", "III", "IV", "V", "VI", "VII"};
  for (int d = 0; d < 7; d++)
    if (r == names[d]) c.degree = d;
  for (const char *fig : {"7", "65", "43", "42"})
    if (s.find(fig) != string::npos) c.has7 = true;
  c.skip = (c.degree < 0);
  return c;
}

int degreeOf(const Key &k, const Pitch &p) { return (p.step - k.tonicStep + 7) % 7; }
bool isChordTone(const Key &k, const Chord &c, const Pitch &p) {
  int rel = (degreeOf(k, p) - c.degree + 7) % 7;
  return rel == 0 || rel == 2 || rel == 4 || (c.has7 && rel == 6);
}
int leadingAcc(const Key &k) {
  int ltStep = (k.tonicStep + 6) % 7;
  int want = (kSemi[k.tonicStep] + k.tonicAcc + 11) % 12;
  int d = (want - kSemi[ltStep] + 12) % 12;
  return d > 6 ? d - 12 : d;
}
bool isLT(const Key &k, const Pitch &p) {
  return degreeOf(k, p) == 6 && p.acc == leadingAcc(k);
}
bool isDominant(const Chord &c) { return c.degree == 4 || c.degree == 6; }

// Parse
struct Score {
  Key key;
  vector<Chord> chords;
  vector<Pitch> v[4];
};

bool parseFile(const string &path, Score &sc, string &err) {
  ifstream in(path);
  if (!in) { err = "cannot open " + path; return false; }
  string line, tok;
  while (getline(in, line)) {
    size_t c0 = line.find_first_not_of(" \t"), colon = line.find(':');
    if (c0 == string::npos || line[c0] == '#' || colon == string::npos)
      continue;
    string tag = line.substr(c0, colon - c0);
    istringstream ss(line.substr(colon + 1));
    if (tag == "key") {
      string tonic, mode;
      Pitch p;
      ss >> tonic >> mode;
      if (!parsePitch(tonic + "4", p)) { err = "bad key: " + tonic; return false; }
      sc.key = {p.step, p.acc, mode == "minor"};
    } else if (tag == "chords") {
      while (ss >> tok) sc.chords.push_back(parseRoman(tok));
    } else {
      int v = 0;
      while (v < 4 && vn(v) != tag) ++v;
      if (v == 4) { err = "unknown line tag: " + tag; return false; }
      for (Pitch p; ss >> tok; sc.v[v].push_back(p))
        if (!parsePitch(tok, p)) { err = "bad pitch: " + tok; return false; }
    }
  }
  if (sc.chords.empty()) { err = "no chords line"; return false; }
  for (int v = 0; v < 4; v++)
    if (sc.v[v].size() != sc.chords.size()) {
      err = vn(v) + " note count != chord count";
      return false;
    }
  return true;
}

// Rules
struct Violation {
  size_t col;
  string rule, detail;
};
vector<Violation> found;
void flag(size_t i, const string &rule, const string &detail) {
  found.push_back({i, rule, detail});
}

const int kLo[4] = {60, 55, 48, 40}, kHi[4] = {79, 74, 67, 60};

void checkVertical(const Score &sc, size_t i) {
  const Key &k = sc.key;
  const Chord &c = sc.chords[i];
  int ltCount = 0;
  for (int v = 0; v < 4; v++) {
    const Pitch &p = sc.v[v][i];
    if (p.midi() < kLo[v] || p.midi() > kHi[v])
      flag(i, "Range", vn(v) + " " + p.name);
    if (v < 2 && p.midi() - sc.v[v + 1][i].midi() > 12)
      flag(i, "Spacing", vn(v) + "-" + vn(v + 1) + " > octave");
    if (v < 3 && p.midi() < sc.v[v + 1][i].midi())
      flag(i, "Crossing", vn(v) + " below " + vn(v + 1));
    if (c.skip) continue;
    if (!isChordTone(k, c, p))
      flag(i, "Non chord tone", vn(v) + " " + p.name + " not in " + c.label);
    if (k.minor && isDominant(c) && degreeOf(k, p) == 6 && !isLT(k, p))
      flag(i, "Unraised LT", vn(v) + " " + p.name + " minor-key " + c.label + " needs raised 7");
    ltCount += isLT(k, p);
  }
  if (ltCount > 1)
    flag(i, "Doubled LT", to_string(ltCount) + " voices on the leading tone");
}

void checkHorizontal(const Score &sc, size_t i) {
  const Key &k = sc.key;
  const Chord &pc = sc.chords[i - 1], &c = sc.chords[i];
  bool chk = !pc.skip && !c.skip;
  for (int v = 0; v < 4; v++) {
    const Pitch &a = sc.v[v][i - 1], &b = sc.v[v][i];
    auto [st, se] = ivl(a, b);
    string mv = vn(v) + " " + a.name + "->" + b.name;
    if (v > 0 && b.midi() > sc.v[v - 1][i - 1].midi())
      flag(i, "Overlap", vn(v) + " above previous " + vn(v - 1));
    if (v < 3 && b.midi() < sc.v[v + 1][i - 1].midi())
      flag(i, "Overlap", vn(v) + " below previous " + vn(v + 1));
    if (abs(st) == 1 && abs(se) == 3)
      flag(i, "Augmented second", mv);
    if (chk && (v == 0 || v == 3) && isLT(k, a) && isDominant(pc) &&
        c.degree == 0 && !(st == 1 && se == 1))
      flag(i, "LT resolution", mv + " outer-voice LT must rise to tonic");
  }
  for (int hi = 0; hi < 3; hi++)
    for (int lo = hi + 1; lo < 4; lo++) {
      int t1 = perfectType(sc.v[lo][i - 1], sc.v[hi][i - 1]),
          t2 = perfectType(sc.v[lo][i], sc.v[hi][i]);
      int mh = sc.v[hi][i].midi() - sc.v[hi][i - 1].midi(),
          ml = sc.v[lo][i].midi() - sc.v[lo][i - 1].midi();
      if (t1 && t1 == t2 && mh && ml && (mh > 0) == (ml > 0))
        flag(i, t1 == 5 ? "Parallel fifths" : "Parallel octaves", vn(hi) + "-" + vn(lo));
    }
}

// Final Report
int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "usage: satb file.satb\n";
    return 2;
  }
  Score sc;
  string err;
  if (!parseFile(argv[1], sc, err)) {
    cerr << "error: " << err << "\n";
    return 2;
  }
  for (size_t i = 0; i < sc.chords.size(); i++) {
    if (sc.chords[i].skip)
      cerr << "warning: chord " << i + 1 << " '" << sc.chords[i].label
           << "' harmony not checked\n";
    checkVertical(sc, i);
    if (i > 0) checkHorizontal(sc, i);
  }
  for (const auto &x : found)
    printf("chord %zu (%s) %-15s %s\n", x.col + 1, sc.chords[x.col].label.c_str(),
           x.rule.c_str(), x.detail.c_str());
  printf("%zu issue(s)\n", found.size());
  return found.empty() ? 0 : 1;
}