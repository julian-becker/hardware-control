#pragma once
#include <utility>


namespace cst {
    namespace {
    
        template <typename T, T val> struct
        val_t {
            using type = T;
            static constexpr T value = val;
        };
        
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
            constexpr pair(ElemT1 elem1, ElemT2 elem2)
                : elem1(elem1)
                , elem2(elem2)
            {}
            
            constexpr pair(const pair& other)
                : elem1(other.elem1)
                , elem2(other.elem2)
            {}
            
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
        
        

        
        template <int SIZE, typename ElemT> class
        array {
            const ElemT vals[SIZE];
        public:
            template <typename...TS>
            constexpr array(TS...vals)
                : vals{vals...}
            {}
            
            template <int Index>
            constexpr ElemT operator [](int_t<Index>) const {
                return vals[Index];
            }

            constexpr ElemT operator [](int index) const {
                return vals[index];
            }

            constexpr ElemT operator [](val<int> index) const {
                return vals[index];
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
        
        template <int SIZE, typename KeyT, typename ValueT> class
        map {
            using ElemT = pair<KeyT,ValueT>;
            const array<SIZE, ElemT> entries;
            
            constexpr val<int> index_of(KeyT key, int index = 0) const {
                return entries[index][0_cst] == key
                       ? val<int>(index)
                       : index_of(key,index+1);
            }
            

            
            
        public:
            template <int...IndexPack>
            constexpr map<SIZE+1,KeyT,ValueT>
            aux_construct(detail::seq<IndexPack...>, ElemT newElem) const {
                return map<SIZE+1,KeyT,ValueT>{ entries[IndexPack]..., newElem };
            }
            
        public:
            template <typename...TS>
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
        };
            
        
    }
}