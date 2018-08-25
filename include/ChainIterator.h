#pragma once

#include <boost/hana.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/variant.hpp>

#include <functional>

template <typename T_Value, typename T_IteratorsTpl>
class ChainIterator : public boost::iterator_facade<
                          ChainIterator<T_Value, T_IteratorsTpl>,
                          T_Value,
                          boost::forward_traversal_tag> {

    using ChainIteratorType = ChainIterator<T_Value, T_IteratorsTpl>;

  public:
    ChainIterator(const T_IteratorsTpl& beginTpl)
        : m_iteratorsTpl(beginTpl)
        , m_section(0)
        , m_maxSection(boost::hana::size(beginTpl))
    {
        m_maxSection--;
    }

    ChainIteratorType& begin()
    {
        return *this;
    }

    ChainIteratorType end()
    {
        auto tmp = *this;
        tmp.m_section = tmp.m_maxSection;

        size_t sectionI = 0;
        boost::hana::for_each(tmp.m_iteratorsTpl, [&tmp, &sectionI](auto& iterators) {
            auto& begin = boost::hana::at_c<0>(iterators);
            auto& end = boost::hana::at_c<1>(iterators);

            if (sectionI != tmp.m_section) {
                sectionI++;
                return;
            }

            begin = end;
        });

        return tmp;
    }

  private:
    void increment()
    {
        size_t sectionI = 0;
        bool incrementSection = false;
        boost::hana::for_each(
            m_iteratorsTpl, [this, &sectionI, &incrementSection](auto& iterators) {
                auto& begin = boost::hana::at_c<0>(iterators);
                auto& end = boost::hana::at_c<1>(iterators);

                if (sectionI != m_section) {
                    sectionI++;
                    return;
                }

                begin++;
                sectionI++;

                if (m_section == m_maxSection && begin == end) {
                    return;
                }

                if (begin == end) {
                    incrementSection = true;
                }
            });

        if (incrementSection) {
            m_section++;
        }
    }

    bool equal(const ChainIteratorType& other) const
    {
        bool isEqual = false;

        callOnCurrentIterator([&other, &isEqual](auto& sectionI, auto& begin, auto& end) {
            if (other.m_section == sectionI) {
                auto& otherBegin
                    = boost::hana::at_c<0>(boost::hana::at(other.m_iteratorsTpl, sectionI));
                auto& otherEnd
                    = boost::hana::at_c<0>(boost::hana::at(other.m_iteratorsTpl, sectionI));

                isEqual = begin == otherBegin && end == otherEnd;
            }
        });

        return isEqual;
    }

    typename std::decay<T_Value>::type& dereference() const
    {
        const typename std::decay<T_Value>::type* value;
        callOnCurrentIterator([&value](auto, auto& begin, auto&) { value = &(*begin); });

        return const_cast<typename std::decay<T_Value>::type&>(*value);
    }

    template <class T_Callable> void callOnCurrentIterator(const T_Callable& callable)
    {
        const_cast<ChainIterator*>(this)->callOnCurrentIterator(callable);
    }

    template <class T_Callable> void callOnCurrentIterator(const T_Callable& callable) const
    {
        boost::hana::fold_left(
            m_iteratorsTpl, m_iteratorsTpl, [this, &callable](auto state, auto& iterators) {
                auto sectionI = boost::hana::minus(
                    boost::hana::size(m_iteratorsTpl), boost::hana::size(state));

                auto isCurrentSection = m_section == sectionI;

                if (isCurrentSection) {
                    callable(
                        sectionI, boost::hana::at_c<0>(iterators), boost::hana::at_c<1>(iterators));
                }
                return boost::hana::drop_front(state);
            });
    }

  private:
    T_IteratorsTpl m_iteratorsTpl;
    std::size_t m_section;
    std::size_t m_maxSection;

    friend class boost::iterator_core_access;
};

template <typename... T_Ranges> struct FirstRange {
    using type = decltype(*(boost::hana::front(boost::hana::tuple<T_Ranges...>{}).begin()));
};

template <typename... T_Ranges> constexpr auto toIterators(const T_Ranges&... ranges) -> auto
{
    return boost::hana::transform(
        boost::hana::make_tuple(std::ref(ranges)...), [](const auto& range) {
            return boost::hana::make_tuple(range.get().begin(), range.get().end());
        });
}

template <typename... T_Ranges> auto makeChainIterator(const T_Ranges&... ranges) -> auto
{
    auto iteratorsTpl = toIterators(ranges...);

    return ChainIterator<
        typename FirstRange<T_Ranges...>::type,
        decltype(iteratorsTpl)>{ iteratorsTpl };
}
