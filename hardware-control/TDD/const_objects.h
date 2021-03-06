#pragma once
#include <utility>
#include <type_traits>
#include <iterator>
#include <array>

namespace cst {
    namespace {
    
        template <typename T, T val> struct
        val_t {
            using type = T;
            static constexpr T value = val;
        };
        
        template <typename T, T val> constexpr T
        val_t<T,val>::value;
        
        template <int N> using int_t = val_t<int,N>;
        
        template <typename T1, T1 N1, typename T2, T2 N2>
        constexpr val_t<decltype(N1+N2),N1+N2>
        operator + (val_t<T1,N1>, val_t<T2,N2>) {
            return val_t<decltype(N1+N2),N1+N2>{};
        }
        
        template <typename T1, T1 N1, typename T2, T2 N2>
        constexpr val_t<decltype(N1-N2),N1-N2>
        operator - (val_t<T1,N1>, val_t<T2,N2>) {
            return val_t<decltype(N1-N2),N1-N2>{};
        }
        
        template <typename T1, T1 N1>
        constexpr val_t<T1,-N1>
        operator - (val_t<T1,N1>) {
            return val_t<T1,-N1>{};
        }
        
        
        template <typename T1, T1 N1, typename T2, T2 N2>
        constexpr decltype(N1 == N2)
        operator == (val_t<T1,N1>, val_t<T2,N2>) {
            return N1 == N2;
        }
        
        template <typename T1, T1 N1, typename T2, T2 N2>
        constexpr decltype(N1 != N2)
        operator != (val_t<T1,N1>, val_t<T2,N2>) {
            return N1 != N2;
        }
        
        namespace detail {
            template <int N, int P> struct
            raise_to_the_n {
                static constexpr int value = N*raise_to_the_n<N,P-1>::value;
            };
            
            template <int N> struct
            raise_to_the_n<N, 0> {
                static constexpr int value = 1;
            };

            static_assert(raise_to_the_n<3, 0>::value == 1,"");
            static_assert(raise_to_the_n<3, 1>::value == 3,"");
            static_assert(raise_to_the_n<3, 2>::value == 9,"");
            static_assert(raise_to_the_n<3, 3>::value == 27,"");
            
            template <char C, char...CS> struct
            build_int_from_str {
                static constexpr int value
                    = (int)(C-'0')*raise_to_the_n<10,sizeof...(CS)>::value
                    + build_int_from_str<CS...>::value;
            };
            
            template <char C> struct
            build_int_from_str<C> {
                static constexpr int value = (int)(C-'0');
            };
            
            static_assert(build_int_from_str<'0'>::value == 0,"");
            static_assert(build_int_from_str<'1','2','3'>::value == 123,"");
            static_assert(build_int_from_str<'1','2','3','4'>::value == 1234,"");
            
        }
        
        template <char...CS>
        constexpr auto operator "" _cst() -> int_t<detail::build_int_from_str<CS...>::value> {
            return int_t<detail::build_int_from_str<CS...>::value> {};
        }
        
        static constexpr unsigned long nibble_from_hex(char c, unsigned order) {
            if(c >= '0' && c <= '9')
               return (unsigned long)((unsigned long)c-(unsigned long)'0') << (order << 2ul);
            else
               return (unsigned long)(10ul + ((unsigned long)c | 32ul)-(unsigned long)'a') << (order << 2u);
        }

        static constexpr unsigned char make_byte(char c1, char c2) {
            return static_cast<unsigned char>(nibble_from_hex(c1,1u) | nibble_from_hex(c2, 0u));
        }
        
        class uuid {
        public:
            const unsigned long  data1;
            const unsigned short data2;
            const unsigned short data3;
            const unsigned char  data4[8u];
            
        
            constexpr uuid(const char (&str)[37u])
                : data1(
                    nibble_from_hex( str[0u], 7u ) |
                    nibble_from_hex( str[1u], 6u ) |
                    nibble_from_hex( str[2u], 5u ) |
                    nibble_from_hex( str[3u], 4u ) |
                    nibble_from_hex( str[4u], 3u ) |
                    nibble_from_hex( str[5u], 2u ) |
                    nibble_from_hex( str[6u], 1u ) |
                    nibble_from_hex( str[7u], 0u )
                  )
                , data2(
                    nibble_from_hex( str[9u],  3u ) |
                    nibble_from_hex( str[10u], 2u ) |
                    nibble_from_hex( str[11u], 1u ) |
                    nibble_from_hex( str[12u], 0u )
                  )
                , data3(
                    nibble_from_hex( str[14u], 3u ) |
                    nibble_from_hex( str[15u], 2u ) |
                    nibble_from_hex( str[16u], 1u ) |
                    nibble_from_hex( str[17u], 0u )
                  )
                // Data 4
                , data4 {
                    make_byte(str[19u], str[20u]),
                    make_byte(str[21u], str[22u]),
                    make_byte(str[24u], str[25u]),
                    make_byte(str[26u], str[27u]),
                    make_byte(str[28u], str[29u]),
                    make_byte(str[30u], str[31u]),
                    make_byte(str[32u], str[33u]),
                    make_byte(str[34u], str[35u])
                }
            {}

            constexpr friend bool operator != (const uuid& a, const uuid& b) {
                return !(a == b);
            }
            
            constexpr friend bool operator == (const uuid& a, const uuid& b) {
                return a.data1 == b.data1
                    && a.data2 == b.data2
                    && a.data3 == b.data3
                    && a.data4[0u] == b.data4[0u]
                    && a.data4[1u] == b.data4[1u]
                    && a.data4[2u] == b.data4[2u]
                    && a.data4[3u] == b.data4[3u]
                    && a.data4[4u] == b.data4[4u]
                    && a.data4[5u] == b.data4[5u]
                    && a.data4[6u] == b.data4[6u]
                    && a.data4[7u] == b.data4[7u];
            }
        };

        constexpr uuid operator"" _uuid( char const* const str, size_t const size ) {
            if(size!=36 || str[8u] != '-' || str[13u] != '-' || str[23u] != '-')
                throw "malformed uuid!";
            return uuid({
                str[ 0u], str[ 1u], str[ 2u], str[ 3u], str[ 4u], str[ 5u], str[ 6u], str[ 7u], str[ 8u],
                str[ 9u], str[10u], str[11u], str[12u], str[13u],
                str[14u], str[15u], str[16u], str[17u], str[18u],
                str[19u], str[20u], str[21u], str[22u], str[23u],
                str[24u], str[25u], str[26u], str[27u], str[28u], str[29u], str[30u], str[31u], str[32u], str[33u], str[34], str[35], '\0'
            });
        }
        
        /// a constexpr value wrapper class
        template <typename ElementType> class
        val {
            const ElementType v;
            
        public:
            constexpr val(ElementType v)
                : v(v)
            {}

            template <int N>
            constexpr val(int_t<N>)
                : v(N)
            {}
            
            constexpr operator ElementType () const {
                return v;
            }
            
            template <typename T>
            constexpr operator val<T> () const {
                return val_t<ElementType, v>{};
            }
        };
                
        template <typename T1, T1 N1, typename T2>
        constexpr decltype(N1 == std::declval<T2>())
        operator == (val_t<T1,N1>, val<T2> v2) {
            return N1 == v2;
        }

        template <typename T1, typename T2, T2 N2>
        constexpr decltype(std::declval<T1>() == N2)
        operator == (val<T1> v1, val_t<T2,N2>) {
            return v1 == N2;
        }
        
        
        template <typename T1, T1 N1, typename T2>
        constexpr decltype(N1 != std::declval<T2>())
        operator != (val_t<T1,N1>, val<T2> v2) {
            return N1 != v2;
        }

        template <typename T1, typename T2, T2 N2>
        constexpr decltype(std::declval<T1>() != N2)
        operator != (val<T1> v1, val_t<T2,N2>) {
            return v1 != N2;
        }

        
        template <typename ElemT1, typename ElemT2> class
        pair {
            const ElemT1 elem1;
            const ElemT2 elem2;
            
        public:
            constexpr pair()
                : elem1{}
                , elem2{}
            {}

            constexpr pair(ElemT1 elem1, ElemT2 elem2)
                : elem1(elem1)
                , elem2(elem2)
            {}
            
            constexpr pair(const pair& other)
                : elem1(other.elem1)
                , elem2(other.elem2)
            {}
            
            constexpr ElemT1& first() { return elem1; }
            constexpr const ElemT1& first() const { return elem1; }

            constexpr ElemT2& second() { return elem2; }
            constexpr const ElemT2& second() const { return elem2; }
            
            template <int Index>
            constexpr std::enable_if_t<Index == 0, ElemT1> operator [](int_t<Index>) const {
                return elem1;
            }

            template <int Index>
            constexpr std::enable_if_t<Index == 1, ElemT2> operator [](int_t<Index>) const {
                return elem2;
            }
        };
        
        
        
        template <typename...ElemTS> class
        tuple {
        public:
            constexpr tuple() {}
        };
        
        template <typename ElemT> class
        tuple<ElemT> {
            const ElemT head;
            
        public:
            constexpr tuple(ElemT head)
                : head(head)
            {}

            template <int Index>
            constexpr std::enable_if_t<Index == 0, ElemT> operator [](int_t<Index>) const {
                return head;
            }
        };
        
        
        namespace detail {
            template <typename T> struct
            provides_type {
                using type = T;
            };
            
            template <int N, typename T, typename...TS> struct
            type_at : provides_type<typename type_at<N-1,TS...>::type> {};
            
            template <typename T, typename...TS> struct
            type_at<0,T,TS...> : provides_type<T> {};

            template <typename T, typename...TS> struct
            type_at<-1,T,TS...> {};


            /// equals the type T_N at index N in the given typelist T == SOME_TYPE<T0,T1,T2,T3,T5...>
            template <int INDEX, typename...TS> using
            type_at_t = typename type_at<INDEX, TS...>::type;
        }
        
        
        
        template <typename ElemT, typename...ElemTS> class
        tuple<ElemT, ElemTS...> {
            const ElemT head;
            const tuple<ElemTS...> rest;
            
        public:
            constexpr tuple(ElemT head, ElemTS...rest)
                : head(head)
                , rest(rest...)
            {}
            
            template <int Index>
            constexpr std::enable_if_t<Index == 0, ElemT>
            operator [](int_t<Index>) const {
                return head;
            }
            
            template <int Index>
            constexpr std::enable_if_t<(Index > 0), detail::type_at_t<Index-1, ElemTS...>>
            operator [](int_t<Index> i) const {
                return rest[int_t<Index-1>{}];
            }
        };
        
        
        namespace detail {
            template<class T> using Invoke = typename T::type;

            template<int...> struct seq{ using type = seq; };

            template<class S1, class S2> struct concat;

            template<int... I1, int... I2>
            struct concat<seq<I1...>, seq<I2...>>
              : seq<I1..., (sizeof...(I1)+I2)...>{};

            template<class S1, class S2>
            using Concat = Invoke<concat<S1, S2>>;

            template<int N> struct gen_seq;
            template<int N> using GenSeq = Invoke<gen_seq<N>>;

            template<int N>
            struct gen_seq : Concat<GenSeq<N/2>, GenSeq<N - N/2>>{};

            template<> struct gen_seq<0> : seq<>{};
            template<> struct gen_seq<1> : seq<0>{};
        }
        

        
        template <int SIZE, typename ElemT> class
        array {
            ElemT vals[SIZE];
            
            template <int...IndexPack, typename = void>
            constexpr array(detail::seq<IndexPack...>, const ElemT (&vals)[SIZE])
                : vals{vals[IndexPack]...}
            {}

            template <int...IndexPack, typename = void>
            constexpr array(detail::seq<IndexPack...>)
                : vals{(IndexPack,ElemT{})...}
            {}
            
        public:
            template <typename T, typename...TS, typename = std::enable_if_t<!std::is_same<T,detail::gen_seq<SIZE>>::value>>
            constexpr array(T v, TS...vals)
                : vals{v, vals...}
            {}

            constexpr array(const ElemT (&vals)[SIZE] )
                : array(detail::gen_seq<SIZE>{}, vals)
            {}

            constexpr array()
                : array(detail::gen_seq<SIZE>{})
            {}
            
            template <int Index>
            constexpr ElemT& operator [](int_t<Index>) {
                return vals[Index];
            }

            template <int Index>
            constexpr const ElemT& operator [](int_t<Index>) const {
                return vals[Index];
            }

            constexpr ElemT& operator [](int index) {
                return vals[index];
            }

            constexpr const ElemT& operator [](int index) const {
                return vals[index];
            }

            constexpr ElemT& operator [](val<int> index) {
                return vals[index];
            }

            constexpr const ElemT& operator [](val<int> index) const {
                return vals[index];
            }
            
            using const_iterator = const ElemT*;
            using iterator = ElemT*;
            
            constexpr const_iterator cbegin() const { return &vals[0]; }
            constexpr const_iterator begin() const { return &vals[0]; }
            constexpr iterator begin() { return &vals[0]; }

            constexpr const_iterator cend() const { return &vals[SIZE]; }
            constexpr const_iterator end() const { return &vals[SIZE]; }
            constexpr iterator end() { return &vals[SIZE]; }
        };
        
        
        template <typename T, int SZ>
        std::enable_if_t<std::is_same<T,char>::value,std::ostream>& operator << (std::ostream& ostr, const array<SZ,T>& arr) {
            for(const auto& el : arr)
                ostr << el;
            return ostr;
        }

        template <typename T, int SZ>
        std::enable_if_t<!std::is_same<T,char>::value,std::ostream>&  operator << (std::ostream& ostr, const array<SZ,T>& arr) {
            bool first = true;
            for(const auto& el : arr) {
                ostr << (first ? "[" : ",") << el;
                first = false;
            }
            ostr << "]";
            return ostr;
        }
        
        template <int SIZE, typename KeyT, typename ValueT> class
        map {
            using ElemT = pair<KeyT,ValueT>;
            const array<SIZE, ElemT> entries;
            
            constexpr val<int> index_of(KeyT key, int index = 0) const {
                return entries[index][0_cst] == key
                       ? val<int>(index)
                       : index_of(key,index+1);
            }
            

            
            template <int...IndexPack>
            constexpr map<SIZE+1,KeyT,ValueT>
            aux_construct(detail::seq<IndexPack...>, ElemT newElem) const {
                return map<SIZE+1,KeyT,ValueT>{ entries[IndexPack]..., newElem };
            }
            
        public:
            template <typename...TS,typename = std::enable_if_t<sizeof...(TS)==SIZE>>
            constexpr map(TS...pairs)
                : entries{pairs...}
            {}
            
            constexpr map(const map& other)
                : entries(other.entries)
            {}
            
            constexpr ValueT operator [] (KeyT key) const {
                return entries[index_of(key)][1_cst];
            }
            
            constexpr map<SIZE+1,KeyT,ValueT>
            insert(KeyT key, ValueT value) const {
                return aux_construct(detail::gen_seq<SIZE>{}, ElemT(key,value));
            }
            
            using iterator = pair<KeyT,ValueT>*;
            using const_iterator = pair<KeyT,ValueT> const *;
            
            constexpr iterator begin() { return &entries[0]; }
            constexpr const_iterator begin() const { return &entries[0]; }
            constexpr iterator end() { return &entries[SIZE]; }
            constexpr const_iterator end() const { return &entries[SIZE]; }
            
            
        };
        
        
        template <int SZ,typename KeyT, typename ValueT>
        std::ostream& operator << (std::ostream& ostr, const map<SZ,KeyT,ValueT>& m) {
            bool first = true;
            for(const auto& kv_pair : m) {
                ostr << (first? "{" : ", ") << kv_pair[0_cst] << ": " << kv_pair[1_cst];
                first = false;
            }
            ostr << "}";
            return ostr;
        }
            
        
    }
}