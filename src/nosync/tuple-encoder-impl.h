// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__TUPLE_ENCODER_IMPL_H
#define NOSYNC__TUPLE_ENCODER_IMPL_H

#include <array>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>


namespace nosync
{

namespace tuple_encoder_impl
{

template<typename T, typename Encoder, typename OutputIt>
struct encoder
{
public:
    constexpr encoder(const Encoder &enc);

    constexpr OutputIt operator()(const T &value, OutputIt out) const;

private:
    const Encoder &enc;
};


template<typename Encoder, typename OutputIt, typename ...EE>
class encoder<std::tuple<EE...>, Encoder, OutputIt>
{
public:
    constexpr encoder(const Encoder &enc);

    constexpr OutputIt operator()(const std::tuple<EE...> &value, OutputIt out) const;

private:
    template<std::size_t Idx>
    constexpr std::enable_if_t<(Idx + 1 < sizeof...(EE)), OutputIt> encode_from_index(const std::tuple<EE...> &value, OutputIt out) const;

    template<std::size_t Idx>
    constexpr std::enable_if_t<(Idx + 1 == sizeof...(EE)), OutputIt> encode_from_index(const std::tuple<EE...> &value, OutputIt out) const;

    const Encoder &enc;
};


template<typename E, typename Encoder, typename OutputIt>
struct encoder<std::tuple<E>, Encoder, OutputIt>
{
public:
    constexpr encoder(const Encoder &enc);

    constexpr OutputIt operator()(const std::tuple<E> &value, OutputIt out) const;

private:
    const Encoder &enc;
};


template<typename Encoder, typename OutputIt>
class encoder<std::tuple<>, Encoder, OutputIt>
{
public:
    constexpr encoder(const Encoder &);

    constexpr OutputIt operator()(const std::tuple<> &, OutputIt out) const;
};


template<typename T1, typename T2, typename Encoder, typename OutputIt>
struct encoder<std::pair<T1, T2>, Encoder, OutputIt>
{
public:
    constexpr encoder(const Encoder &enc);

    constexpr OutputIt operator()(const std::pair<T1, T2> &value, OutputIt out) const;

private:
    const Encoder &enc;
};


template<typename E, std::size_t I, typename Encoder, typename OutputIt>
struct encoder<std::array<E, I>, Encoder, OutputIt>
{
public:
    constexpr encoder(const Encoder &enc);

    constexpr OutputIt operator()(const std::array<E, I> &value, OutputIt out) const;

private:
    const Encoder &enc;
};


template<typename SizeCalc>
class encoded_size_collector
{
public:
    constexpr encoded_size_collector(const SizeCalc &size_calc);

    template<typename T>
    constexpr std::size_t operator()(const T &value, std::size_t input_size) const;

private:
    const SizeCalc &size_calc;
};


template<typename T, typename Encoder, typename OutputIt>
constexpr encoder<T, Encoder, OutputIt>::encoder(const Encoder &enc)
    : enc(enc)
{
}


template<typename T, typename Encoder, typename OutputIt>
constexpr OutputIt encoder<T, Encoder, OutputIt>::operator()(const T &value, OutputIt out) const
{
    return enc(value, out);
}


template<typename Encoder, typename OutputIt, typename ...EE>
constexpr encoder<std::tuple<EE...>, Encoder, OutputIt>::encoder(const Encoder &enc)
    : enc(enc)
{
}


template<typename Encoder, typename OutputIt, typename ...EE>
constexpr OutputIt encoder<std::tuple<EE...>, Encoder, OutputIt>::operator()(const std::tuple<EE...> &value, OutputIt out) const
{
    return encode_from_index<0>(value, out);
}


template<typename Encoder, typename OutputIt, typename ...EE>
template<std::size_t Idx>
constexpr std::enable_if_t<(Idx + 1 < sizeof...(EE)), OutputIt> encoder<std::tuple<EE...>, Encoder, OutputIt>::encode_from_index(const std::tuple<EE...> &value, OutputIt out) const
{
    return encode_from_index<Idx + 1>(value, encode_tuple(std::get<Idx>(value), enc, out));
}


template<typename Encoder, typename OutputIt, typename ...EE>
template<std::size_t Idx>
constexpr std::enable_if_t<(Idx + 1 == sizeof...(EE)), OutputIt> encoder<std::tuple<EE...>, Encoder, OutputIt>::encode_from_index(const std::tuple<EE...> &value, OutputIt out) const
{
    return encode_tuple(std::get<Idx>(value), enc, out);
}


template<typename E, typename Encoder, typename OutputIt>
constexpr encoder<std::tuple<E>, Encoder, OutputIt>::encoder(const Encoder &enc)
    : enc(enc)
{
}


template<typename E, typename Encoder, typename OutputIt>
constexpr OutputIt encoder<std::tuple<E>, Encoder, OutputIt>::operator()(const std::tuple<E> &value, OutputIt out) const
{
    return encode_tuple(std::get<0>(value), enc, out);
}


template<typename Encoder, typename OutputIt>
constexpr encoder<std::tuple<>, Encoder, OutputIt>::encoder(const Encoder &)
{
}

template<typename Encoder, typename OutputIt>
constexpr OutputIt encoder<std::tuple<>, Encoder, OutputIt>::operator()(const std::tuple<> &, OutputIt out) const
{
    return out;
}



template<typename T1, typename T2, typename Encoder, typename OutputIt>
constexpr encoder<std::pair<T1, T2>, Encoder, OutputIt>::encoder(const Encoder &enc)
    : enc(enc)
{
}


template<typename T1, typename T2, typename Encoder, typename OutputIt>
constexpr OutputIt encoder<std::pair<T1, T2>, Encoder, OutputIt>::operator()(const std::pair<T1, T2> &value, OutputIt out) const
{
    return encode_tuple(std::get<1>(value), enc, encode_tuple(std::get<0>(value), enc, out));
}


template<typename E, std::size_t I, typename Encoder, typename OutputIt>
constexpr encoder<std::array<E, I>, Encoder, OutputIt>::encoder(const Encoder &enc)
    : enc(enc)
{
}


template<typename E, std::size_t I, typename Encoder, typename OutputIt>
constexpr OutputIt encoder<std::array<E, I>, Encoder, OutputIt>::operator()(const std::array<E, I> &value, OutputIt out) const
{
    for (std::size_t i = 0; i < value.size(); ++i) {
        out = encode_tuple(value[i], enc, out);
    }

    return out;
}


template<typename SizeCalc>
constexpr encoded_size_collector<SizeCalc>::encoded_size_collector(const SizeCalc &size_calc)
    : size_calc(size_calc)
{
}


template<typename SizeCalc>
template<typename T>
constexpr std::size_t encoded_size_collector<SizeCalc>::operator()(const T &value, std::size_t input_size) const
{
    return input_size + size_calc(value);
}


template<typename T, typename Encoder, typename OutputIt>
constexpr OutputIt encode_tuple_impl(const T &value, const Encoder &enc, OutputIt out)
{
    return encoder<T, Encoder, OutputIt>(enc)(value, out);
};

}


template<typename T, typename Encoder, typename OutputIt>
constexpr OutputIt encode_tuple(const T &value, const Encoder &enc, OutputIt out)
{
    return tuple_encoder_impl::encode_tuple_impl(value, enc, out);
};


template<typename T, typename SizeCalculator>
constexpr std::size_t calc_encoded_tuple_size(const T &value, const SizeCalculator &size_calc)
{
    return encode_tuple(value, tuple_encoder_impl::encoded_size_collector<SizeCalculator>(size_calc), static_cast<std::size_t>(0));
}

}

#endif /* NOSYNC__TUPLE_ENCODER_IMPL_H */
