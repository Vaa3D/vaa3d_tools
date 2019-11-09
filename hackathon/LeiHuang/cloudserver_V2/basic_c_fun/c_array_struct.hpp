/*
 * c_array_struct.hpp
 *
 *  Created on: Jan 8, 2011
 *      Author: cmbruns
 */

#ifndef C_ARRAY_STRUCT_HPP_
#define C_ARRAY_STRUCT_HPP_

/*! Light wrapper for C-arrays, to make them easier
 * to expose in python.
 *
 * Runtime performance should not be affected:
 * No virtual methods; no additional data members;
 * All methods are inline.
 */
template<class ELT, int SIZE>
class c_array {
public:
    typedef ELT value_type;
    typedef int size_type;
    typedef int index_type;
    typedef ELT* iterator;
    typedef ELT const * const_iterator;

    // indexing operators are the most important thing to retain
    // Problem - gcc on Mac makes operator[] ambiguous when cast operator is included
    // const ELT& operator[](index_type i) const {return data[i];}
    // ELT& operator[](index_type i) {return data[i];}

    // allow implicit conversion to pointer
    // Hopefully this resolves index operator too.
    operator ELT*() {return data;}
    operator const ELT*() const {return data;}

    static size_type size() {return SIZE;}

    iterator begin() {return &data[0];}
    const_iterator begin() const {return &data[0];}
    iterator end() {return &data[SIZE];}
    const_iterator end() const {return &data[SIZE];}

private:
    ELT data[SIZE];
};

#endif /* C_ARRAY_STRUCT_HPP_ */
