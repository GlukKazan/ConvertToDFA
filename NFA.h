#pragma once
#include "Common.h"
#include "StaticAssert.h"

namespace RegExp {

// Конструирование NFA по выражениям вида: Q< D< C<'a'>, E< C<'b'>, C<'c'> > >, 0, 1> - (a|bc)?
template <char Chr>
struct C
{ typedef typename Edge<NullType,0,1,Chr> Result;
  enum {Count = 2};
};

// 0 -> 0
// x -> x + (count-1)
template <int X, int N>
struct ConvA { enum { Result = (X==1) ? (X+N-1) : X }; };

// 1 -> 1
// x -> x + (count)
template <int X, int N>
struct ConvB { enum { Result = (X==1) ? 1 : (X+N) }; };

template <class A, class B>
struct EImpl
{ private:
    typedef typename Map<typename A::Result, A::Count, NullType, ConvA>::Result A1;
    typedef typename Map<typename B::Result, A::Count, NullType, ConvB>::Result B1;
  public:
    typedef typename Append<A1,B1>::Result Result;
    enum {Count = A::Count+B::Count};
};
template <class T1, class T2, class T3 = NullType, class T4 = NullType, class T5 = NullType>
struct E: public EImpl<T1, E<T2,T3,T4,T5> > {};
template <class T1, class T2>
struct E<T1,T2,NullType,NullType,NullType>: public EImpl<T1,T2> {};

// x -> x + N
template <int X, int N>
struct Add { enum { Result = X+N }; };

template <class A, class B>
struct DImpl
{ private:
    typedef typename Append<typename Map<typename A::Result, 2, NullType, Add>::Result,
                            typename Map<typename B::Result, A::Count+2, NullType, Add>::Result
                           >::Result N0;
    typedef typename   Edge<N0,0,2> N1;
    typedef typename   Edge<N1,0,A::Count+2> N2;
    typedef typename   Edge<N2,3,1> N3;
  public:
    typedef typename   Edge<N3,A::Count+3,1> Result;
    enum {Count = A::Count+B::Count+2};
};
template <class T1, class T2, class T3 = NullType, class T4 = NullType, class T5 = NullType>
struct D: public DImpl<T1, D<T2,T3,T4,T5> > {};
template <class T1, class T2>
struct D<T1,T2,NullType,NullType,NullType>: public DImpl<T1,T2> {};

template <class T, int Min = 0, int Max = 0> struct Q
{ Q() {STATIC_ASSERT(Min<=Max, Q_Spec);}
  private:
    typedef typename Map<typename T::Result, T::Count, NullType, ConvA>::Result A1;
    typedef typename Map<typename Q<T,Min,Max-1>::Result, T::Count, NullType, ConvB>::Result B1;
  public:
    typedef typename Edge<typename Append<A1,B1>::Result,0,T::Count> Result;
    enum {Count = T::Count+Q<T,Min,Max-1>::Count};
};
template <class T, int N> struct Q<T,N,N>
{ private:
    typedef typename Map<typename T::Result, T::Count, NullType, ConvA>::Result A1;
    typedef typename Map<typename Q<T,N-1,N-1>::Result, T::Count, NullType, ConvB>::Result B1;
  public:
    typedef typename Append<A1,B1>::Result Result;
    enum {Count = T::Count+Q<T,N-1,N-1>::Count};
};
template <class T> struct Q<T,1,1>: public T {};
template <class T>
struct Q<T,0,0>
{ private:
    typedef typename Edge<typename Map<typename T::Result, 2, NullType, Add>::Result,0,2> N0;
    typedef typename Edge<N0,3,1> N1;
    typedef typename Edge<N1,3,2> N2;
  public:
    typedef typename Edge<N2,0,1> Result;
    enum {Count = T::Count+2};
};
template <class T>
struct Q<T,1,0>
{ public:
    typedef typename Edge<typename T::Result,1,0> Result;
    enum {Count = T::Count};
};
template <class T>
struct Q<T,0,1>
{ public:
    typedef typename Edge<typename T::Result,0,1> Result;
    enum {Count = T::Count};
};

};
