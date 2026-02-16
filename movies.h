#include <string>

class Movie {
public:
    std::string name;
    int score;

    // move name instead of copying it
    Movie(std::string name, int score) : name(std::move(name)), score(score) {}

    std::string getName() const;
    int getScore() const;

    friend bool operator<(const Movie& lhs, const Movie& rhs);
    friend bool operator>(const Movie& lhs, const Movie& rhs);
    friend bool operator==(const Movie& lhs, const Movie& rhs);
};
