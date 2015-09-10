

class Rules {
    std::vector<size_t> target_offsets;
    std::vector<size_t> source_position;
    std::vector<size_t> length
    size_t current_position = 0;
};

template<class T>
class GrowableSdslVec {
    T vec;
    size_t size;

    GrowableSdslVec(): size(0) {
    }

    void push_back(T value) {
        if (size == vec.size()) {
            auto cap = size + (size - 1) / 2 + 1;
            vec.resize(cap);
        }
        vec[size] = std::move(value);
        size++;
    }

};
