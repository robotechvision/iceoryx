#ifndef IOX_BINDING_C_EXCLUSIVITY_CHECK_HPP
#define IOX_BINDING_C_EXCLUSIVITY_CHECK_HPP

// #define USE_EXCLUSIVITY_CHECK
#define USE_EXCLUSIVITY_CHECK2

#ifdef USE_EXCLUSIVITY_CHECK2
#define CHECK_EXCL2 {ExclusivityCheck __ec{(void *)self};
#define UNCHECK_EXCL2 }
#else
#define CHECK_EXCL2
#define UNCHECK_EXCL2
#endif

#ifdef USE_EXCLUSIVITY_CHECK
#define CHECK_EXCL {ExclusivityCheck __ec{};
#define UNCHECK_EXCL }
#else
#define CHECK_EXCL
#define UNCHECK_EXCL
#endif

void checkExclusive(void *ptr);
void uncheckExclusive(void *ptr);
class ExclusivityCheck {
public:
    ExclusivityCheck() : ptr((void *)16806) {checkExclusive(ptr);};
    template <typename T>
    ExclusivityCheck(T ptr) : ptr((void *)ptr) {checkExclusive(this->ptr);};
    ~ExclusivityCheck() {uncheckExclusive(ptr);};
private:
    void *ptr;
};

#endif