#ifndef RollingMean_hpp
#define RollingMean_hpp

#include <deque>

class RollingMean {
public:
    RollingMean(size_t windowSize) : windowSize(windowSize), sum(0.0) {}

    void add(double value) {
        values.push_back(value);
        sum += value;
        if (values.size() > windowSize) {
            sum -= values.front();
            values.pop_front();
        }
    }

    double mean() const {
        return values.empty() ? 0.0 : sum / values.size();
    }

private:
    size_t windowSize;
    std::deque<double> values;
    double sum;
};

#endif /* RollingMean_hpp */
