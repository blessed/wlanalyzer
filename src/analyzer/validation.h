#ifndef VALIDATION_H
#define VALIDATION_H

#include <functional>
#include <QString>

namespace validation {
    /**
     * A common functor type for string validation
     * @param input QString to be validated
     * @param error QString with contextual help/error message that can be used
     *              e.g. in tooltips. This is implementation dependent and can be
     *              used even in case of successful validation
     * @return true if validation is successful
     */
    typedef std::function<bool(const QString& input, QString& error)> StrCheckFn_t;
    enum State_t {Empty, Invalid, Valid};
};
#endif // VALIDATION_H
