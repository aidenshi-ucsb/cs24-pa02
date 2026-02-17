// Winter'24
// Instructor: Diba Mirza
// Student name: Aiden Shi
#pragma GCC optimize("Ofast,unroll-loops")
#pragma GCC target("avx512f,avx2,bmi,bmi2,lzcnt,popcnt")
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <cstring>
#include <algorithm>
#include <limits.h>
#include <iomanip>
#include <set>
#include <queue>
#include <sstream>
#include <array>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

#include "utilities.h"
#include "movies.h"

void parse_line(std::string_view &line, std::string_view &movie_name, unsigned int &movie_rating);

void main_part1(char *movie_filepath) {
  int mf_fd = open(movie_filepath, O_RDONLY);
  if (mf_fd < 0) { cerr << "Could not open file " << movie_filepath; exit(1); }
  struct stat mf_st;
  fstat(mf_fd, &mf_st);
  size_t mf_size = mf_st.st_size;
  const char* mf_buffer = (const char*)mmap(nullptr, mf_size, PROT_READ, MAP_PRIVATE, mf_fd, 0);
  close(mf_fd);

  // why vector? O(1) push & O(n log n) sort
  // which is equivilent to the O(n log n) complexity
  // for n inserts into a red black tree. plus we get
  // the advantage of cache coherency
  std::vector<Movie> movies;
  const char* curr = mf_buffer;
  const char* end = mf_buffer + mf_size;

  std::string_view movie_name;
  unsigned int movie_rating;
  while (curr < end) {
    const char* next_nl = (const char*)memchr(curr, '\n', end - curr);
    if (!next_nl) next_nl = end;
    std::string_view line(curr, next_nl - curr);
    if (!line.empty()) {
      parse_line(line, movie_name, movie_rating);
      movies.emplace_back(movie_name, movie_rating);
    }
    curr = next_nl + 1;
  }

  std::sort(movies.begin(), movies.end(),
            [](const Movie& a, const Movie& b) { return a.name < b.name; });

  std::string out;
  out.reserve(1 << 20);
  for (const auto& movie : movies) {
    out += movie.name;
    out += ", ";
    out += std::to_string(movie.score >> 4);
    out += '.';
    out += char((movie.score & 0xf) + '0');
    out += '\n';
  }
  if(write(STDOUT_FILENO, out.data(), out.size()))
    ;

  _Exit(0);
}

// out of all of the titles, what's the lowest
// and highest character of all characters?
// then we find `highest - lowest + 1` to store prefixes.
// it happens to be ' ' for lowest and '~' for highest
// BUT we only care about prefix length when calculating
// so within the first 3 characters, it's ' ' and 'z'
// respectively
// see anaylsis.py
#define CHAR_RANGE (90 + 2)
#define NIL_CHR (' ' - 1)

// well it's not *exactly* a trie but its close
typedef std::vector<const Movie*>* SparseTrie[CHAR_RANGE][CHAR_RANGE][CHAR_RANGE];

static SparseTrie trie = {};

// instead of initialzing all 729k vectors, we only
// initialize them when needed, keeping them 0 allocated
// otherwise.
inline std::vector<const Movie*>& trie_get_or_create(SparseTrie& t, int a, int b, int c) {
  auto& cell = t[a][b][c];
  if (!cell) {
    cell = new std::vector<const Movie*>();
    cell->reserve(8);
  }
  return *cell;
}


void main_part2(char *movie_filepath, char *prefix_filepath) {
  int mf_fd = open(movie_filepath, O_RDONLY);
  if (mf_fd < 0) { cerr << "Could not open file " << movie_filepath; exit(1); }
  struct stat mf_st;
  fstat(mf_fd, &mf_st);
  const size_t mf_size = mf_st.st_size;
  const char* mf_buffer = (const char*)mmap(nullptr, mf_size, PROT_READ, MAP_PRIVATE, mf_fd, 0);
  close(mf_fd);

  // bucket based approach (inspired by radix sort)
  // depending on the score we sort it into a bucket, then iterate backwards
  std::vector<Movie> buckets[0xa1 * (CHAR_RANGE - 1)];
  // for (auto& b : buckets) b.reserve(16);

  const char* curr = mf_buffer;
  const char* end = mf_buffer + mf_size;

  std::string_view movie_name;
  unsigned int movie_rating;
  while (curr < end) {
    const char* next_nl = (const char*)memchr(curr, '\n', end - curr);
    if (!next_nl) next_nl = end;
    std::string_view line(curr, next_nl - curr);
    if (!line.empty()) {
      parse_line(line, movie_name, movie_rating);
      if (!movie_name.empty()) {
	buckets[movie_rating * (CHAR_RANGE - 1) + (movie_name[0] - ' ')].emplace_back(movie_name, movie_rating);
      }
    }
    curr = next_nl + 1;
  }

  for (auto& bucket : buckets) {
    std::sort(bucket.begin(), bucket.end(), [](const Movie& a, const Movie& b){
      return a.name < b.name;
    });
  }

  for (int s = (0xa1 * (CHAR_RANGE - 1)) - 1; s >= 0; --s) {
    for (const auto& movie : buckets[s]) {
      int size = movie.name.size();
      switch (size) {
      default:
	trie_get_or_create(trie, movie.name[0] - NIL_CHR, movie.name[1] - NIL_CHR, movie.name[2] - NIL_CHR).push_back(&movie);
      case 2:
	trie_get_or_create(trie, movie.name[0] - NIL_CHR, movie.name[1] - NIL_CHR, 0).push_back(&movie);
      case 1:
	trie_get_or_create(trie, movie.name[0] - NIL_CHR, 0, 0).push_back(&movie);
      case 0:
	{}
      }
    }
  }

  int pf_fd = open(prefix_filepath, O_RDONLY);
  if (pf_fd < 0) { cerr << "Could not open file " << prefix_filepath; exit(1); }
  struct stat pf_st;
  fstat(pf_fd, &pf_st);
  size_t pf_size = pf_st.st_size;
  const char* pf_buffer = (const char*)mmap(nullptr, pf_size, PROT_READ, MAP_PRIVATE, pf_fd, 0);
  close(pf_fd);

  std::string out;
  out.reserve(1 << 24);
  std::string best_buffer;
  best_buffer.reserve(1 << 24);

  curr = pf_buffer;
  end = pf_buffer + pf_size;
  while (curr < end) {
    const char* next_nl = (const char*)memchr(curr, '\n', end - curr);
    if (!next_nl) next_nl = end;
    std::string_view line(curr, next_nl - curr);

    if (!line.empty()) {
      std::vector<const Movie*>* cell;

      switch (line.size()) {
      case 3: cell = trie[line[0] - NIL_CHR][line[1] - NIL_CHR][line[2] - NIL_CHR]; break;
      case 2: cell = trie[line[0] - NIL_CHR][line[1] - NIL_CHR][0]; break;
      case 1: cell = trie[line[0] - NIL_CHR][0][0]; break;
      default: __builtin_unreachable(); 
      }

      if (!cell || cell->size() == 0) {
	out += "No movies found with prefix ";
	out += line;
	out += '\n';
      } else {
	for (const auto &movie : *cell) {
	  out += movie->name;
	  out += ", ";
	  out += std::to_string(movie->score >> 4);
	  out += '.';
	  out += char((movie->score & 0xf) + '0');
	  out += '\n';
	}
	out += '\n';

	const Movie* best = (*cell)[0];
	best_buffer += "Best movie with prefix ";
	best_buffer += line;
	best_buffer += " is: ";
	best_buffer += best->name;
	best_buffer += " with rating ";
	best_buffer += std::to_string(best->score >> 4);
	best_buffer += '.';
	best_buffer += char((best->score & 0xf) + '0');
	best_buffer += '\n';
      }

    }

    curr = next_nl + 1;
  }

  // direct write instead of through cout
  out += best_buffer;
  if(write(STDOUT_FILENO, out.data(), out.size()))
    ;

  _Exit(0);
}

int main(int argc, char** argv) {
    ios_base::sync_with_stdio(false);
    // cin.tie(nullptr);
    if (argc < 2){
        cerr << "Not enough arguments provided (need at least 1 argument)." << endl;
        cerr << "Usage: " << argv[ 0 ] << " moviesFilename prefixFilename " << endl;
        exit(1);
    }

    if (argc == 2) main_part1(argv[1]);
    if (argc == 3) main_part2(argv[1], argv[2]);

    _Exit(0);
}

// ~64 mb heap
char HEAP[0x4000000];
char *heap = HEAP;


// custom bump allocator malloc
extern "C" void *malloc(unsigned long amount) {
  void *chunk = (void *)heap;
  // round up to 16 for alignment purposes
  heap += amount + 15 & ~15;
  return chunk;
}

extern "C" void free(void* ptr) {
  // noop
}

/*
  Hello, and sorry to whoever has review my code. I hope its somewhat straight-
  forward, but if you have any questions shoot an email.

  In order to do efficient prefix lookup we can use a Trie data structure, which
  has O(d) where d is the max length of any given prefix. An analysis of the
  prefix provided shows us that prefixes are not longer than 3. Since d is
  constant, we can represent our trie as a 3 dimensional array structure, where
  each index represents an edge in our Trie.

  The downside of this approach is memory usage, since it represents *every*
  possible edge (For all ASCII this would be 256^3). To reduce this, we can find
  the minimum and maximum character of all titles (see analysis.py) since edges
  outside of this range cannot possibly	exist. Because of this optimization,
  starts with lookups are O(1). Repeated m times for each prefix, and k movies
  for each prefix, the time complexity for the lookup portion is O(mk).

  In order to build the trie, I took some inspiration from radix sort. If we
  pre-sort the movies in the order we want and then insert them into a trie, the
  order in the trie will be in the correct order as well. To store all of the
  movies, I decided on a sorted vector. Since the sort is a one time thing, we
  incur a cost of O(n log n), but in order to do a comparison, we must compare
  the strings, the total cost is O(n l log n).

  Combining our 2 steps, we get a final runtime cost of O(nl log(n) + mk).

  > hyperfine --warmup 3 -N \
    './runMovies input_20_random.csv prefix_large.txt' \
    './runMovies input_100_random.csv prefix_large.txt' \
    './runMovies input_1000_random.csv prefix_large.txt' \
    './runMovies input_76920_random.csv prefix_large.txt'
  Benchmark 1: ./runMovies input_20_random.csv prefix_large.txt
    Time (mean ± σ):       1.8 ms ±   0.2 ms    [User: 0.9 ms, System: 0.9 ms]
    Range (min … max):     1.6 ms …   3.0 ms    1427 runs

  Benchmark 2: ./runMovies input_100_random.csv prefix_large.txt
    Time (mean ± σ):       1.9 ms ±   0.2 ms    [User: 0.9 ms, System: 0.8 ms]
    Range (min … max):     1.6 ms …   2.6 ms    1309 runs

  Benchmark 3: ./runMovies input_1000_random.csv prefix_large.txt
    Time (mean ± σ):       2.2 ms ±   0.2 ms    [User: 1.2 ms, System: 0.9 ms]
    Range (min … max):     1.9 ms …   3.5 ms    1265 runs

  Benchmark 4: ./runMovies input_76920_random.csv prefix_large.txt
    Time (mean ± σ):      30.9 ms ±   1.2 ms    [User: 24.6 ms, System: 5.9 ms]
    Range (min … max):    29.0 ms …  34.5 ms    98 runs

  Summary
    ./runMovies input_20_random.csv prefix_large.txt ran
      1.02 ± 0.14 times faster than ./runMovies input_100_random.csv prefix_large.txt
      1.21 ± 0.16 times faster than ./runMovies input_1000_random.csv prefix_large.txt
     16.70 ± 1.79 times faster than ./runMovies input_76920_random.csv prefix_large.txt

  This didn't align with what I expected, so I decided to profile my code.
  Flamegraphs can be found here:
  https://gist.github.com/aidenshi-ucsb/bd8944452610506d02fd39867d2355a2

  Once we look at the graphs it becomes obvious. In our 1000 movie run, 29.15%
  of the runtime is dedicated to `main_part2` and most of the runtime is startup
  costs. In our 76920 movie run, 35.93% of the runtime is dedicated to
  `main_part2`, and while the startup cost has diminished, we see that 27.4% of
  our runtime is now in libc, and also 16.5% of the time is dedicated to sorting
  the list.

  The sorting cost is expected to dominate our runtime which we do begin to see
  as the list grows large, but we also see the cost of reading a large file start
  to dominate our costs as well. While we've optimized file reading as much as
  possible by mmap-ing it directly into memory to reduce round trips to the
  kernel, the inherit cost of copying from the disk to memory becomes apparent.

 */

void parse_line(std::string_view &line, std::string_view &movie_name, unsigned int &movie_rating) {
  int comma_index = line.find_last_of(",");
  int size = line.size();
  // There are 3 cases we need to handle
  // abc
  // X.Y -> c * 1  + a * 10
  //  10 -> c * 1  + b * 100
  //   X -> c * 10
  // we can really use b as a differentiator since it will be ',' or '.' or '1' based on the case
  movie_rating =
      ((int)(line[size - 2] == ',') * 15 + 1) * (line[size - 1] - '0') +
      (int)(line[size - 2] == '1') * 0xa0 +
      (int)(line[size - 2] == '.') * (line[size - 3] - '0') * 16;

  int quoted = (int)(line[0] == '\"');
  int start = quoted, end = comma_index - quoted;
  movie_name = std::string_view(&line[start], end - start);
}
