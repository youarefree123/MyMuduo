#pragma once

#include <string>

class Timestamp
{
public:
    Timestamp()
        :microseconds_since_epoch_(0){}
  
    explicit Timestamp( size_t micro_seconds_arg ) 
        :microseconds_since_epoch_( micro_seconds_arg ){}

    ~Timestamp() = default;
    size_t microseconds_since_epoch() const { return microseconds_since_epoch_; }

    static Timestamp Now();
    std::string ToString() const;

    Timestamp( const Timestamp& lhs ) {
        microseconds_since_epoch_ = lhs.microseconds_since_epoch();
    }

    Timestamp& operator= ( const Timestamp& lhs ) {
        this->microseconds_since_epoch_ = lhs.microseconds_since_epoch();
        return *this;
    }
    // 用法 string str =  (string)time;
    operator std::string() const { return ToString(); }

private:
    size_t microseconds_since_epoch_; 
};

inline bool operator< (const Timestamp& lhs,const Timestamp& rhs ) {
    return lhs.microseconds_since_epoch() < rhs.microseconds_since_epoch();
}

inline bool operator== (const Timestamp& lhs,const Timestamp& rhs ) {
    return lhs.microseconds_since_epoch() == rhs.microseconds_since_epoch();
}


inline std::ostream& operator<< (std::ostream& os, const Timestamp& lhs ) {
    os<<lhs.ToString();
    return os;
}