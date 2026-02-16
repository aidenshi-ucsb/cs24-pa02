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
using namespace std;

#include "utilities.h"
#include "movies.h"

bool parse_line(string &line, string &movie_name, unsigned int &movie_rating);


int main_part1(char *movie_filepath) {
  ifstream movie_file(movie_filepath);

  if (movie_file.fail()) {
    cerr << "Could not open file " << movie_filepath;
    exit(1);
  }

  // why vector? O(1) push & O(n log n) sort
  // which is equivilent to the O(n log n) complexity
  // for n inserts into a red black tree. plus we get
  // the advantage of cache coherency
  std::vector<Movie> movies;
  std::string line, movie_name;
  unsigned int movie_rating;

  while (getline(movie_file, line) && parse_line(line, movie_name, movie_rating)){
    movies.push_back(Movie(movie_name, movie_rating));
  }

  // movieFile.close(); // we don't need to do this since the operating system will do it for us

  std::sort(movies.begin(), movies.end(),
            [](Movie a, Movie b) { return a.name < b.name; });

  for (const auto& movie : movies) {
    cout << movie.name << ", " << movie.score / 10 << '.' << movie.score % 10 << '\n';
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
typedef std::vector<const Movie*> PrefixTrie[CHAR_RANGE][CHAR_RANGE][CHAR_RANGE];

int main_part2(char *movie_filepath, char *prefix_filepath) {
  ifstream movie_file(movie_filepath);
  if (movie_file.fail()) {
    cerr << "Could not open file " << movie_filepath;
    exit(1);
  }
  std::vector<Movie> movies;
  // 90^3 * 32 bytes or approximately 23 megabytes
  // so we need to do a heap allocation 
  auto prefix_trie = new PrefixTrie;
  std::string line, movie_name;
  unsigned int movie_rating;

  while (getline(movie_file, line) && parse_line(line, movie_name, movie_rating)){
    movies.push_back(Movie(movie_name, movie_rating));
  }

  // we want the default sort here, but greatest first
  std::sort(movies.begin(), movies.end(), std::greater<Movie>());

  for (const auto& movie : movies) {
    int size = movie.name.size();
    // we actually want passthrough here
    switch (size) {
    default:
      prefix_trie[movie.name[0] - NIL_CHR][movie.name[1] - NIL_CHR][movie.name[2] - NIL_CHR].push_back(&movie);
    case 2:
      prefix_trie[movie.name[0] - NIL_CHR][movie.name[1] - NIL_CHR][NIL_CHR].push_back(&movie);
    case 1:
      prefix_trie[movie.name[0] - NIL_CHR][NIL_CHR][NIL_CHR].push_back(&movie);
    case 0:
      {}
    }
  }

  ifstream prefix_file (prefix_filepath);

  if (prefix_file.fail()) {
    cerr << "Could not open file " << prefix_filepath;
    exit(1);
  }

  string best_buffer = "";
  while (getline (prefix_file, line)) {
    if (line.empty()) continue;
    int size = line.size();

    std::vector<const Movie*>* res;

    switch (size) {
    case 3: res = &prefix_trie[line[0] - NIL_CHR][line[1] - NIL_CHR][line[2] - NIL_CHR]; break;
    case 2: res = &prefix_trie[line[0] - NIL_CHR][line[1] - NIL_CHR][NIL_CHR]; break;
    case 1: res = &prefix_trie[line[0] - NIL_CHR][NIL_CHR][NIL_CHR]; break;
    default: __builtin_unreachable(); 
    }

    if (res->size() == 0) {
      std::cout << "No movies found with prefix "<< line << '\n';
    } else {
      for (const auto &movie : *res) {
	std::cout << movie->name << ", " << movie->score / 10 << '.' << movie->score % 10 << '\n';
      }

      std::cout << '\n';

      const Movie* best = (*res)[0];
      best_buffer += "Best movie with prefix " + line + " is: " + best->name + " with rating " + (char)(best->score / 10 + '0') + '.' + (char)(best->score % 10 + '0') + '\n';
    }
  }

  std::cout << best_buffer << '\n';

  return 0;
}

int main(int argc, char** argv) {
    if (argc < 2){
        cerr << "Not enough arguments provided (need at least 1 argument)." << endl;
        cerr << "Usage: " << argv[ 0 ] << " moviesFilename prefixFilename " << endl;
        exit(1);
    }

    if (argc == 2) return main_part1(argv[1]);
    if (argc == 3) return main_part2(argv[1], argv[2]);


    //  For each prefix,
    //  Find all movies that have that prefix and store them in an appropriate data structure
    //  If no movie with that prefix exists print the following message
    cout << "No movies found with prefix "<<"<replace with prefix>" << endl;

    //  For each prefix,
    //  Print the highest rated movie with that prefix if it exists.
    cout << "Best movie with prefix " << "<replace with prefix>" << " is: " << "replace with movie name" << " with rating " << std::fixed << std::setprecision(1) << "replace with movie rating" << endl;

    return 0;
}

/* Add your run time analysis for part 3 of the assignment here as commented block*/

bool parse_line(string &line, string &movie_name, unsigned int &movie_rating) {
  int comma_index = line.find_last_of(",");
  movie_name = line.substr(0, comma_index);
  // assuming the range of ratings is 0.0 - 9.9

  // note to self: if this assumption is not allowed,
  // instead use the fact that '.' - '0' is negative
  // to trigger an overflow resulting in 10 being the
  // highest possible score (and integer)
  movie_rating = (line[comma_index + 1] - '0') * 10;
  if (comma_index + 3 < (int)line.size())
    movie_rating += (line[comma_index + 3] - '0');
  if (movie_name[0] == '\"') {
    movie_name = movie_name.substr(1, movie_name.length() - 2);
  }
  return true;
}
