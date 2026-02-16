#include "movies.h"

bool operator<(const Movie& lhs, const Movie& rhs) {
    if (lhs.score < rhs.score) return true;
    if (lhs.score > rhs.score) return false;
    
    return lhs.name > rhs.name;
}

bool operator>(const Movie& lhs, const Movie& rhs) {
    if (lhs.score > rhs.score) return true;
    if (lhs.score < rhs.score) return false;
    
    return lhs.name < rhs.name;
}

bool operator==(const Movie& lhs, const Movie& rhs) {
    return (lhs.score == rhs.score) && (lhs.name == rhs.name);
}
