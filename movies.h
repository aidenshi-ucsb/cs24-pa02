#include <string>

class Movie {
public:
    std::string_view name;
    int score;

    // move name instead of copying it
    Movie(std::string_view name, int score) : name(name), score(score) {}

    std::string getName() const;
    int getScore() const;

    friend bool operator<(const Movie& lhs, const Movie& rhs);
    friend bool operator>(const Movie& lhs, const Movie& rhs);
    friend bool operator==(const Movie& lhs, const Movie& rhs);
};
