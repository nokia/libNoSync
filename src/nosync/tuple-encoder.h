// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__TUPLE_ENCODER_H
#define NOSYNC__TUPLE_ENCODER_H

#include <cstddef>


namespace nosync
{

/*!
Encode tuple-like object using specified encoder.

The function traverses all elements of tuple-like object of type T, recursively
processing elements which are also tuple-like objects, and for each non
tuple-like object it calls provided encoder (function object) passing two
parameters to it: a) the object to encode, b) output iterator to use for encoded
data.

The result is output iterator value returned by the last encoder call or
unmodified iterator if the encoder was never called.

Tuple-like objects are objecs of types: std::array<>, std::pair<>, std::tuple<>.

The encoder may support multiple types via overloaded operator().
*/
template<typename T, typename Encoder, typename OutputIt>
constexpr OutputIt encode_tuple(const T &value, const Encoder &enc, OutputIt out);


/*!
Calculate encoded size of tuple-like object using specified "sizeof" calculator.

The function recursively traverses all elements of tuple-like object of type T,
calculating encoded size of each non tuple-like element using provided size
calculator (size_calc is called with sub-element as the only parameter).

The result is a sum of values returned by the size calculator for all visited
sub-elements.

The size calculator may support multiple types via overloaded operator().

If value is constexpr and SizeCalc::operator() is constexpr for all visited
types then the function call is evaluated at compile time.

\see encode_tuple()
*/
template<typename T, typename SizeCalc>
constexpr std::size_t calc_encoded_tuple_size(const T &value, const SizeCalc &size_calc);

}

#include <nosync/tuple-encoder-impl.h>

#endif /* NOSYNC__TUPLE_ENCODER_H */
