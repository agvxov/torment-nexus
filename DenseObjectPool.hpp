#include <vector>
#include <cstddef>
#include <utility>
#include <iterator>
#include <cassert>

template <class T>
class DenseObjectPool {
public:
    using value_type = T;
    using container_type = std::vector<T>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    class mut_iter {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        mut_iter() = default;

        reference operator*() const {
            assert(pool_ != nullptr);
            assert(index_ < pool_->size());
            return (*pool_)[index_];
        }

        pointer operator->() const {
            return &(**this);
        }

        mut_iter& operator++() {
            if (skip_next_increment_) {
                skip_next_increment_ = false;
                return *this;
            }

            if (pool_ == nullptr) {
                return *this;
            }

            if (index_ < pool_->size()) {
                ++index_;
            }

            return *this;
        }

        mut_iter operator++(int) {
            mut_iter tmp = *this;
            ++(*this);
            return tmp;
        }

        void kill_current() {
            assert(pool_ != nullptr);
            assert(index_ < pool_->size());
            pool_->kill(index_);
            skip_next_increment_ = true;
        }

        friend bool operator==(const mut_iter& a, const mut_iter& b) {
            return a.pool_ == b.pool_ && a.index_ == b.index_;
        }

        friend bool operator!=(const mut_iter& a, const mut_iter& b) {
            return !(a == b);
        }

    private:
        friend class DenseObjectPool;

        mut_iter(DenseObjectPool* pool, std::size_t index)
            : pool_(pool), index_(index) {}

        DenseObjectPool* pool_ = nullptr;
        std::size_t index_ = 0;
        bool skip_next_increment_ = false;
    };

    DenseObjectPool() = default;

    explicit DenseObjectPool(std::size_t capacity) {
        storage_.reserve(capacity);
    }

    void reserve(std::size_t capacity) {
        storage_.reserve(capacity);
    }

    void resize(std::size_t n) {
        storage_.resize(n);
    }

    template <class... Args>
    T& emplace_back(Args&&... args) {
        storage_.emplace_back(std::forward<Args>(args)...);
        return storage_.back();
    }

    void kill(int i) {
        assert(i >= 0);
        kill(static_cast<std::size_t>(i));
    }

    void kill(std::size_t i) {
        assert(i < storage_.size());
        storage_[i] = std::move(storage_.back());
        storage_.pop_back();
    }

    void kill(T* t) {
        assert(t != nullptr);
        assert(!storage_.empty());

        T* base = storage_.data();
        assert(t >= base && t < base + storage_.size());

        std::size_t i = static_cast<std::size_t>(t - base);
        kill(i);
    }

    std::size_t size() const {
        return storage_.size();
    }

    bool empty() const {
        return storage_.empty();
    }

    void clear() {
        storage_.clear();
    }

    T& operator[](std::size_t i) {
        return storage_[i];
    }

    const T& operator[](std::size_t i) const {
        return storage_[i];
    }

    iterator begin() { return storage_.begin(); }
    iterator end() { return storage_.end(); }
    const_iterator begin() const { return storage_.begin(); }
    const_iterator end() const { return storage_.end(); }
    const_iterator cbegin() const { return storage_.cbegin(); }
    const_iterator cend() const { return storage_.cend(); }

    mut_iter mut_begin() {
        return mut_iter(this, 0);
    }

    mut_iter mut_end() {
        return mut_iter(this, storage_.size());
    }

    T* data() {
        return storage_.data();
    }

    const T* data() const {
        return storage_.data();
    }

private:
    container_type storage_;
};
