
#ifndef slider_h
#define slider_h


class Slider final
{
public:
    Slider(int64_t i_current, int64_t i_min, int64_t i_max) : _min(i_min), _max(i_max) {
        _cursor = this->clamp(i_current);
    }
public:
    int64_t value() const { return _cursor; }
public:
    int64_t operator << (int64_t i_offset) { return this->shl(i_offset); }
    int64_t operator >> (int64_t i_offset) { return this->shr(i_offset); }
public:
    int64_t shl(int64_t i_offset) { return (_cursor = this->clamp(_cursor - i_offset)); } // 汇编，SHift Left
    int64_t shr(int64_t i_offset) { return (_cursor = this->clamp(_cursor + i_offset)); } // 汇编，SHift Right
private:
    inline int64_t clamp(int64_t i) const {
        if(i < _min) { return _min; } else if (i > _max) { return _max; } return i;
    }
private:
    int64_t _cursor;
private:
    const int64_t _min;
    const int64_t _max;
};

#endif /* slider_h */
