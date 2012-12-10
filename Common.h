#pragma once

namespace RegExp {

// Вспомогательные шаблоны
struct NullType {static void Dump(void) {printf("\n");}};
template <int N> struct Int2Type {enum {Result = N};};
template <bool cond, class A, class B> struct If {typedef A Result;};
template <class A, class B> struct If<false,A,B> {typedef B Result;};

// Элемент множества
template <int N, class T>
struct Set
{ enum {Value = N};
  typedef T Next;
  static void Dump(void) {printf("%d ",N);T::Dump();}
};

// Узел графа
template <class T, int Src, int Dst, char Chr = 0>
struct Node
{ enum { Source  = Src,
         Dest    = Dst,
         Char    = Chr
       };
  typedef T Next;
  static void Dump(void) {printf("%3d -%c-> %3d\n",Src,Chr,Dst);T::Dump();}
};

// Элемент списка состояний
template <int N, class S, class T>
struct StateList
{ enum {Value = N};
  typedef S State;
  typedef T Next;
  static void Dump(void) {printf("%5d: ",N);S::Dump();T::Dump();}
};
template <int src, int dst, int a, class S, class T>
struct StateListEx: public StateList<dst,S,T>
{ enum {Src  = src,
        Dst  = dst,
        Move = a};
  static void Dump(void) {printf("%5d(%c): ",dst,a);S::Dump();T::Dump();}
};

// Проверка вхождения элемента в список
template <int N, class T> struct Exist;
template <int N> struct Exist<N,NullType> {enum {Result = false};};
template <int N, class T> struct Exist<N,Set<N,T> > {enum {Result = true};};
template <int N, int M, class T> struct Exist<N,Set<M,T> > {enum {Result = Exist<N,T>::Result};};
template <int N, class S, class T> struct Exist<N, StateList<N,S,T> > {enum {Result = true;};};
template <int N, int M, class S, class T> struct Exist<N, StateList<M,S,T> > {enum {Result = Exist<N,T>::Result};};
template <int N, int Src, int a, class S, class T> struct Exist<N, StateListEx<Src,N,a,S,T> > {enum {Result = true;};};
template <int N, int Src, int Dst, int a, class S, class T> struct Exist<N, StateListEx<Src,Dst,a,S,T> > {enum {Result = Exist<N,T>::Result};};
template <int N, class T> struct NotExist {enum {Result = !Exist<N,T>::Result};};

// Проверка включения A в B
template <class A, class B> struct Include;
template <int N, class T, class B>
struct Include<Set<N,T>,B> {enum {Result = Exist<N,B>::Result && Include<T,B>::Result};};
template <class B> struct Include<NullType,B> {enum {Result = true};};

// Проверка равенства списков
template <class A, class B> struct Equ {enum {Result = Include<A,B>::Result && Include<B,A>::Result};};

// Слияние списков (A => B)
template <class A, class B> struct Append;
template <class T> struct Append<NullType,T> {typedef T Result;};
template <int N, class T, class B> struct Append<Set<N,T>,B> {typedef typename Append<T,Set<N,B> >::Result Result;};
template <int S, int D, int C, class T, class B> struct Append<Node<T,S,D,C>,B> {typedef typename Append<T,Node<B,S,D,C> >::Result Result;};
template <int N, class S, class T, class B> struct Append<StateList<N,S,T>,B> {typedef typename Append<T,StateList<N,S,B> >::Result Result;};
template <int Src, int Dst, int a, class S, class T, class B> struct Append<StateListEx<Src,Dst,a,S,T>,B> {typedef typename Append<T,StateList<Dst,S,B> >::Result Result;};
template <class A, class B> struct AppendSafe;
template <class T> struct AppendSafe<NullType,T> {typedef T Result;};
template <int Src, int Dst, int a, class S, class T, class B> struct AppendSafe<StateListEx<Src,Dst,a,S,T>,B> {typedef typename AppendSafe<T,StateListEx<Src,Dst,a,S,B> >::Result Result;};

// Включение в множество без повторов
template <int N, class T, class R> struct InsSet;
template <int N, class R> struct InsSet<N,NullType,R> {typedef typename Set<N,R> Result;};
template <int N, class T, class R> struct InsSet<N,Set<N,T>,R> {typedef typename InsSet<N,T,R>::Result Result;};
template <int N, int M, class T, class R> struct InsSet<N,Set<M,T>,R> {typedef typename InsSet<N,T,Set<M,R> >::Result Result;};

// Включение в граф без повторов
template <int S, int D, int C, class T, class R> struct InsNode;
template <int S, int D, int C, class R> struct InsNode<S,D,C,NullType,R> {typedef typename Node<R,S,D,C> Result;};
template <int S, int D, int C, class T, class R> struct InsNode<S,D,C,Node<T,S,D,C>,R> {typedef typename InsNode<S,D,C,T,R>::Result Result;};
template <int S, int D, int C, int s, int d, int c, class T, class R> struct InsNode<S,D,C,Node<T,s,d,c>,R> {typedef typename InsNode<S,D,C,T,Node<R,s,d,c> >::Result Result;};

// Включение в список состояний без повторов
template <int N, class S, class T, class R> struct InsState;
template <int N, class S, class R> struct InsState<N,S,NullType,R> {typedef typename StateList<N,S,R> Result;};
template <int N, class S, class T, class R> struct InsState<N,S,StateList<N,S,T>,R> {typedef typename InsState<N,S,T,R>::Result Result;};
template <int N, int n, class S, class s, class T, class R> struct InsState<N,S,StateList<n,s,T>,R> {typedef typename InsState<N,S,T,StateList<n,s,R> >::Result Result;};

// Слияние списков без повторов (A => B)
template <class A, class B> struct Join;
template <class B> struct Join<NullType,B> {typedef B Result;};
template <int N, class T, class B> struct Join<Set<N,T>,B> {typedef typename Join<T,typename InsSet<N,B,NullType>::Result>::Result Result;};
template <int S, int D, int C, class T, class B> struct Join<Node<T,S,D,C>,B> {typedef typename Join<T,typename InsNode<S,D,C,B,NullType>::Result>::Result Result;};
template <int N, class S, class T, class B> struct Join<StateList<N,S,T>,B> {typedef typename Join<T,typename InsState<N,S,B,NullType>::Result>::Result Result;};
template <int Src, int Dst, int a, class S, class T, class B> struct Join<StateListEx<Src,Dst,a,S,T>,B> {typedef typename Join<T,typename InsState<Dst,S,B,NullType>::Result>::Result Result;};

// Преобразование элементов списка
template <class T, int V, class R, template <int,int> class F> struct Map;
template <int V, class R, template <int,int> class F> struct Map<NullType,V,R,F> {typedef R Result;};
template <int N, class T, int V, class R, template <int,int> class F> struct Map<Set<N,T>,V,R,F>
{ typedef typename Map<T,V,Set<F<N,V>::Result,R>,F>::Result Result;
};
template <class T, int S, int D, int C, int V, class R, template <int,int> class F> struct Map<Node<T,S,D,C>,V,R,F>
{ typedef typename Map<T,V,Node<R,F<S,V>::Result,F<D,V>::Result,C>,F>::Result Result;
};

};
