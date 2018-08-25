#pragma once

// STL
#include <map>
#include <vector>
#include <memory>

// HANA
#include <boost/hana.hpp>

namespace hana = boost::hana;

namespace detail {

/**
 * @brief Type definition for map of maps.
 *
 */
template <typename... T_Tail> struct MapOfMaps;

template <typename T_Key, typename T_Value> struct MapOfMaps<T_Key, T_Value> {
    using type = std::map<T_Key, T_Value>;
};

template <typename T_Key, typename... T_Tail> struct MapOfMaps<T_Key, T_Tail...> {
    using type = typename std::map<T_Key, typename MapOfMaps<T_Tail...>::type>;
};

/**
 * @brief Collects all values of a map subtree into the values container and its
 *        corresponding keys into the key container
 *
 */
template <int KeysLeft> struct SubTreeValues {

    template <typename T_Map, typename T_ValuesCT, typename T_KeysCT, typename T_Tuple>
    void operator()(T_Map& map, T_ValuesCT& values, T_KeysCT& keys, T_Tuple& tuple)
    {
        auto begin = map.begin();
        auto end = map.end();
        for (auto it = begin; it != end; ++it) {
            auto nextTuple = hana::concat(tuple, hana::make_tuple(it->first));
            SubTreeValues<KeysLeft - 1>()(it->second, values, keys, nextTuple);
        }
    }

    template <typename T_Map, typename T_ValuesCT, typename T_Keys>
    void operator()(T_Map& map, T_ValuesCT& values, T_Keys& keys)
    {
        auto begin = map.begin();
        auto end = map.end();
        for (auto it = begin; it != end; ++it) {
            auto t = hana::make_tuple(it->first);
            SubTreeValues<KeysLeft - 1>()(it->second, values, keys, t);
        }
    }
};

template <> struct SubTreeValues<0> {

    template <typename T_Value, typename T_ValuesCT, typename T_KeysCT, typename T_Tuple>
    void operator()(T_Value& value, T_ValuesCT& values, T_KeysCT& keys, T_Tuple& tuple)
    {
        keys.push_back(tuple);
        values.push_back(value);
    }
};

/**
 * @brief Traverse through a map of map of ... and
 *        return the value of the last key.
 *
 */
auto at([](auto& map, const auto& key) -> auto& { return map[key]; });

template <typename T_Map, typename T_Keys> auto& traverse(T_Map& map, T_Keys const& keys)
{
    return hana::fold_left(keys, map, at);
}

} // namespace detail

/**
 * @brief A map with multiple keys, implemented by cascading several
 *        std::maps.
 *
 * The MultiKeyMap has the goal to provide the look and feel of
 * a usual std::map by the at and () methods. Furthermore, it
 * provides the possibility to retrieve the values of subtrees
 * by specifying not all keys by the values method.
 *
 */
template <typename... T_Args> class MultiKeyMap {

    using ArgsTpl = hana::tuple<T_Args...>;
    using LastKey = decltype(hana::back(ArgsTpl()));
    using Value = typename std::decay<LastKey>::type;
    using ValueCt = std::vector<Value>;
    using KeysTpl = decltype(hana::take_front(
        ArgsTpl(), hana::minus(hana::size(ArgsTpl()), hana::size(hana::make_tuple(1)))));
    using KeysTplCt = std::vector<KeysTpl>;

  public:
    MultiKeyMap()
    {
    }

    /**********************************************************************
     * Iterator
     **********************************************************************/
    class Iterator {
      public:
        Iterator(const std::shared_ptr<ValueCt>& values, const typename ValueCt::iterator& it)
            : m_values(values)
            , m_it(it)
        {
        }

        Iterator(const Iterator&)
        {
        }

        Iterator operator=(const Iterator&)
        {
        }

        ~Iterator()
        {
        }

        Value operator*()
        {
            return *m_it;
        }

        Iterator& operator++()
        {
            m_it++;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            m_it++;
            return tmp;
        }

        bool operator==(const Iterator& iterator)
        {
            return iterator.m_it == m_it;
        }

        bool operator!=(const Iterator& iterator)
        {
            return !(*this == iterator);
        }


    private:
        std::shared_ptr<ValueCt> m_values;
        typename ValueCt::iterator m_it;
    };

    template <typename... T_Keys> Iterator begin(const T_Keys... keys)
    {
        auto valuesCt = std::make_shared<ValueCt>();
        KeysTplCt keysTplCt;
        values(*valuesCt, keysTplCt, keys...);

        return {valuesCt, valuesCt->begin()};
    }

    template <typename... T_Keys> Iterator end(const T_Keys... keys)
    {
        auto valuesCt = std::make_shared<ValueCt>();
        KeysTplCt keysTplCt;
        values(*valuesCt, keysTplCt, keys...);

        return {valuesCt, valuesCt->end()};
    }

    /**********************************************************************
     * Element accesses
     **********************************************************************/
    template <typename... T_Keys> Value& operator()(const T_Keys... keys)
    {
        return detail::traverse(multiKeyMap, hana::make_tuple(keys...));
    }

    template <typename... T_Keys> Value& at(const T_Keys... keys)
    {
        return atImpl(hana::make_tuple(keys...));
    }

    /**********************************************************************
     * Modifiers
     **********************************************************************/
    template <typename... T_Keys> bool erase(const T_Keys... keys)
    {
        return eraseImpl(hana::make_tuple(keys...));
    }

    /**********************************************************************
     * Operations
     **********************************************************************/
    template <typename... T_Keys> bool test(const T_Keys... keys)
    {
        return testImpl(hana::make_tuple(keys...));
    }

    void values(Value& values, KeysTplCt& keys)
    {
        valuesImpl(values, keys);
    }

    template <typename... T_Sub_Keys>
    void values(ValueCt& values, KeysTplCt& keys, const T_Sub_Keys... subKeys)
    {
        valuesImpl(values, keys, subKeys...);
    }

    template <typename... T_Keys> Iterator find(const T_Keys... keys)
    {
        auto result = test(keys...);
        if (result == false) {
            return end();
        }

        return begin(keys...);
    }

  private:
    typename detail::MapOfMaps<T_Args...>::type multiKeyMap;

    /***************************************************************************
     * at
     ***************************************************************************/
    template <typename T_KeysTpl> Value& atImpl(const T_KeysTpl keysTuple)
    {
        auto firstKeysSize = hana::int_c<hana::minus(hana::int_c<hana::size(keysTuple)>, 1)>;
        auto firstKeys = hana::take_front_c<firstKeysSize>(keysTuple);
        auto lastKey = hana::back(keysTuple);

        return detail::traverse(multiKeyMap, firstKeys).at(lastKey);
    }

    /***************************************************************************
     * test
     ***************************************************************************/
    template <typename T_KeysTpl> bool testImpl(const T_KeysTpl keysTuple)
    {
        auto firstKeysSize = hana::int_c<hana::minus(hana::int_c<hana::size(keysTuple)>, 1)>;
        auto firstKeys = hana::take_front_c<firstKeysSize>(keysTuple);
        auto lastKey = hana::back(keysTuple);

        auto& subMap = detail::traverse(multiKeyMap, firstKeys);
        auto it = subMap.find(lastKey);
        auto end = subMap.end();

        return (it == end) ? false : true;
    }

    /***************************************************************************
     * find
     ***************************************************************************/
    template <typename T_KeysTpl> bool findImpl(const T_KeysTpl keysTuple)
    {
        auto firstKeysSize = hana::int_c<hana::minus(hana::int_c<hana::size(keysTuple)>, 1)>;
        auto firstKeys = hana::take_front_c<firstKeysSize>(keysTuple);
        auto lastKey = hana::back(keysTuple);

        auto& subMap = detail::traverse(multiKeyMap, firstKeys);
        auto it = subMap.find(lastKey);
        auto end = subMap.end();

        return (it == end) ? false : true;
    }


    /***************************************************************************
     * erase
     ***************************************************************************/
    template <typename T_KeysTpl> bool eraseImpl(const T_KeysTpl keysTuple)
    {
        auto firstKeysSize = hana::int_c<hana::minus(hana::int_c<hana::size(keysTuple)>, 1)>;
        auto firstKeys = hana::take_front_c<firstKeysSize>(keysTuple);
        auto lastKey = hana::back(keysTuple);

        auto& lastMap = detail::traverse(multiKeyMap, firstKeys);
        auto it = lastMap.find(lastKey);
        auto end = lastMap.end();

        if (it == end)
            return false;
        else {
            lastMap.erase(it);
            return true;
        }
    }

    /***************************************************************************
     * values
     ***************************************************************************/
    void valuesImpl(Value& value, KeysTplCt& keys)
    {
        auto keysTupleSize = hana::size(KeysTpl());
        detail::SubTreeValues<keysTupleSize>()(multiKeyMap, value, keys);
    }

    template <typename... T_Sub_Keys>
    void valuesImpl(ValueCt& values, KeysTplCt& keys, const T_Sub_Keys... subKeys)
    {
        auto subKeysTupleSize = hana::size(hana::tuple<T_Sub_Keys...>());
        auto keysTupleSize = hana::size(KeysTpl());
        auto subTreeSize = keysTupleSize - subKeysTupleSize;
        auto tuple = hana::make_tuple(subKeys...);
        detail::SubTreeValues<subTreeSize>()(
            detail::traverse(multiKeyMap, tuple), values, keys, tuple);
    }
};
