#include <string>

class Movie {
public:
    std::string name;
    int score;

    Movie(std::string name, int score) : name(name), score(score) {}

    std::string getName() const;
    int getScore() const;

    friend bool operator<(const Movie& lhs, const Movie& rhs);
    friend bool operator>(const Movie& lhs, const Movie& rhs);
    friend bool operator==(const Movie& lhs, const Movie& rhs);
};
