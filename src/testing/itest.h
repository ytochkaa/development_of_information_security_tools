#ifndef ITEST_H
#define ITEST_H

#include <string>

class ITest {
public:
    virtual ~ITest() = default;
    virtual std::string name() const = 0;
    virtual void run() = 0;
};

#endif // ITEST_H
