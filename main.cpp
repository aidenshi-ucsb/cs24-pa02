// Winter'24
// Instructor: Diba Mirza
// Student name: 
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
using namespace std;

#include "utilities.h"
#include "movies.h"

bool parse_line(std::string_view &line, std::string_view &movie_name, unsigned int &movie_rating);

int main_part1(char *movie_filepath) {
  ifstream movie_file(movie_filepath, std::ios::binary | std::ios::ate);

  if (movie_file.fail()) {
    cerr << "Could not open file " << movie_filepath;
    exit(1);
  }

  std::streamsize size = movie_file.tellg();
  movie_file.seekg(0, std::ios::beg);

  std::string mf_buffer;
  mf_buffer.resize(size);

  movie_file.read(&mf_buffer[0], size);

  // why vector? O(1) push & O(n log n) sort
  // which is equivilent to the O(n log n) complexity
  // for n inserts into a red black tree. plus we get
  // the advantage of cache coherency
  std::vector<Movie> movies;
  int line_start = 0;
  int curr = 0;

  std::string_view movie_name;
  unsigned int movie_rating;
  for (;;) {
    while (!(mf_buffer[curr] == '\n' || mf_buffer[curr] == '\0')) curr++;
    std::string_view line(&mf_buffer[line_start], curr - line_start);
    if (!line.empty()) {
      parse_line(line, movie_name, movie_rating);
      movies.emplace_back(movie_name, movie_rating);
    }
    if (mf_buffer[curr] == '\0') break;
    curr = line_start = curr + 1;
  }

  std::sort(movies.begin(), movies.end(),
            [](const Movie& a, const Movie& b) { return a.name < b.name; });

  for (const auto& movie : movies) {
    cout << movie.name << ", " << movie.score / 10 << '.' << char(movie.score % 10 + '0') << '\n';
  }

  return 0;
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

// instead of initialzing all 729k vectors, we only
// initialize them when needed, keeping them 0 allocated
// otherwise.
inline std::vector<const Movie*>& trie_get_or_create(
    SparseTrie& t, int a, int b, int c) {
  auto& cell = t[a][b][c];
  if (!cell) cell = new std::vector<const Movie*>();
  return *cell;
}

static const std::vector<const Movie*> empty_vec;

int main_part2(char *movie_filepath, char *prefix_filepath) {
  // avoid \r\n translation since that could slow us down
  ifstream movie_file(movie_filepath, std::ios::binary | std::ios::ate);

  if (movie_file.fail()) {
    cerr << "Could not open file " << movie_filepath;
    exit(1);
  }

  // read file into string
  std::streamsize size = movie_file.tellg();
  movie_file.seekg(0, std::ios::beg);

  std::string mf_buffer;
  mf_buffer.resize(size);

  movie_file.read(&mf_buffer[0], size);

  // bucket based approach (inspired by radix sort)
  // depending on the score we sort it into a bucket, then iterate backwards
  std::vector<Movie> buckets[101];
  for (auto& b : buckets) b.reserve(800);

  int line_start = 0;
  int curr = 0;

  std::string_view movie_name;
  unsigned int movie_rating;
  for (;;) {
    while (!(mf_buffer[curr] == '\n' || mf_buffer[curr] == '\0')) curr++;
    std::string_view line(&mf_buffer[line_start], curr - line_start);
    if (!line.empty()) {
      parse_line(line, movie_name, movie_rating);
      buckets[movie_rating].emplace_back(movie_name, movie_rating);
    }
    if (mf_buffer[curr] == '\0') break;
    curr = line_start = curr + 1;
  }

  for (auto& bucket : buckets) {
    std::sort(bucket.begin(), bucket.end(), [](const Movie& a, const Movie& b){
      return a.name < b.name;
    });
  }

  // move them into one global movies vector
  std::vector<Movie> movies;
  movies.reserve(80000);
  for (int s = 100; s >= 0; --s) {
    for (auto& m : buckets[s]) movies.push_back(std::move(m));
  }

  auto& trie = *(SparseTrie*)calloc(1, sizeof(SparseTrie));

  for (const auto& movie : movies) {
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

  ifstream prefix_file(prefix_filepath, std::ios::binary | std::ios::ate);

  if (prefix_file.fail()) {
    cerr << "Could not open file " << prefix_filepath;
    exit(1);
  }

  size = prefix_file.tellg();
  prefix_file.seekg(0, std::ios::beg);

  std::string pf_buffer;
  pf_buffer.resize(size);

  prefix_file.read(&pf_buffer[0], size);

  std::string out;
  out.reserve(1 << 20);
  std::string best_buffer;
  best_buffer.reserve(1 << 18);

  curr = 0;
  line_start = 0;

  for (;;) {
    while (!(pf_buffer[curr] == '\n' || pf_buffer[curr] == '\0')) curr++;
    std::string_view line(&pf_buffer[line_start], curr - line_start);

    if (!line.empty()) {
      std::vector<const Movie*>* cell;

      switch (curr - line_start) {
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
	  out += std::to_string(movie->score / 10);
	  out += '.';
	  out += char(movie->score % 10 + '0');
	  out += '\n';
	}
	out += '\n';

	const Movie* best = (*cell)[0];
	best_buffer += "Best movie with prefix ";
	best_buffer += line;
	best_buffer += " is: ";
	best_buffer += best->name;
	best_buffer += " with rating ";
	best_buffer += std::to_string(best->score / 10);
	best_buffer += '.';
	best_buffer += char(best->score % 10 + '0');
	best_buffer += '\n';
      }

    }

    if (pf_buffer[curr] == '\0') break;
    curr = line_start = curr + 1;
  }

  // direct write instead of through cout
  out += best_buffer;
  if(write(STDOUT_FILENO, out.data(), out.size()))
    ;

  return 0;
}

int main(int argc, char** argv) {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    if (argc < 2){
        cerr << "Not enough arguments provided (need at least 1 argument)." << endl;
        cerr << "Usage: " << argv[ 0 ] << " moviesFilename prefixFilename " << endl;
        exit(1);
    }

    if (argc == 2) return main_part1(argv[1]);
    if (argc == 3) return main_part2(argv[1], argv[2]);

    return 0;
}

/* Add your run time analysis for part 3 of the assignment here as commented block*/

bool parse_line(std::string_view &line, std::string_view &movie_name, unsigned int &movie_rating) {
  int comma_index = line.find_last_of(",");

  int num_index = comma_index + (int)(line[comma_index + 1] == ' ');
  movie_rating = (line[num_index + 1] - '0') * 10;

  if (num_index + 3 < (int)line.size()) movie_rating += (line[num_index + 3] - '0');
  else if (line[num_index + 2] == '0') movie_rating *= 10; // handle 10 case

  int start = 0, end = comma_index;
  if (line[0] == '\"') { start = 1; end--; }
  movie_name = std::string_view(&line[start], end - start);
  return true;
}
