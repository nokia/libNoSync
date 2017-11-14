// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__INTERFACE_TYPE_H
#define NOSYNC__INTERFACE_TYPE_H


namespace nosync
{

/*!
Abstract base type for "interface" classes.

Abstract class which is intended to be used as a  base type for all "interface"
classes, i.e. classes with only pure virtual functions. It makes the class
non-copyable and defines virtual destructor.

It can also be used as static type (e.g. for a function's result) where an
object's type is not important (only e.g. its lifetime)
*/
class interface_type
{
public:
    virtual ~interface_type() = default;

    interface_type(interface_type &&) = delete;
    interface_type(const interface_type &) = delete;
    interface_type &operator=(const interface_type &) = delete;

protected:
    interface_type() = default;
};

}

#endif /* NOSYNC__INTERFACE_TYPE_H */
