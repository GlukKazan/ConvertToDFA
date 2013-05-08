#pragma once
#include "Common.h"

namespace RegExp {

enum CONSTS {
   MAX_FIN_STATE = 9
};

// Реализация DFA
template <class Graph> class DFAImpl;
template <class T, int Src, int Dst, char Chr>
class DFAImpl<Edge<T,Src,Dst,Chr> >: public DFAImpl<typename T>
{ public:
    typedef typename    DFAImpl<typename T>::ResultType ResultType;
    ResultType          Parse(char C)
    {
      if ((State==Src)&&(C==Chr)) {
           State = Dst;
           if (State<MAX_FIN_STATE) {
               State = 0;
               return rtSucceed;
           }
           return rtNotCompleted;
      }
      return DFAImpl<typename T>::Parse(C);
    }
    void Dump(void) {T::Dump();}
};
template <>
class DFAImpl<NullType>
{ public:
    DFAImpl():          State(0) {}
    enum ResultType {
       rtNotCompleted = 0,
       rtSucceed      = 1,
       rtFailed       = 2
    };
    ResultType          Parse(char C)
    {  State = 0;
       return rtFailed;
    }
  protected:
    int                 State;
};

// Вычисление хода (списка состояний) из вершины (При a==0 - e-ход)                                                 *
// N - Узел
// T - Граф
// R - Результирующее состояние
// a - Символ алфавита
template <int N, class T, class R, int a = 0> struct Move;
template <int N, class R, int a> struct Move<N,NullType,R,a> {typedef R Result;};
template <int N, class T, int D, class R, int a> struct Move<N,Edge<T,N,D,a>,R,a>
{ typedef typename Move<N,T,typename AddSet<D,R,NullType>::Result,a>::Result Result;
};
template <int N, int M, class T, int D, class R, int a, int b> struct Move<N,Edge<T,M,D,b>,R,a>
{ typedef typename Move<N,T,R,a>::Result Result;
};

// Фильтрация списка по условию F                                                                                   *
// T - Исходный список (Set, StateListEx)
// С - Значение параметра предиката F
// R - Результирующий список (Set, StateListEx)
// F - Предикат (Exist, NotExist, Important)
template <class T, class C, class R, template <int,class> class F> struct Filter;
template <class C, class R, template <int,class> class F> struct Filter<NullType,C,R,F> {typedef R Result;};
template <int N, class T, class C, class R, template <int,class> class F> struct Filter<Set<N,T>,C,R,F>
{ typedef typename If<F<N,C>::Result,
                      typename Filter<T,C,typename Set<N,R>,F>::Result,
                      typename Filter<T,C,R,F>::Result
                     >::Result Result;
};
template <int Src, int Dst, int a, class S, class T, class C, class R, template <int,class> class F> 
struct Filter<StateListEx<Src,Dst,a,S,T>,C,R,F>
{ typedef typename If<F<Dst,C>::Result,
                      typename Filter<T,C,typename StateListEx<Src,Dst,a,S,R>,F>::Result,
                      typename Filter<T,C,R,F>::Result
                     >::Result Result;
};

// Вычисление e-замыкания                                                                                           *
// T - Начальный список узлов
// G - Граф
// R - Результирующий список узлов
template <class T, class G, class R> struct EClos;
template <class G, class R> struct EClos<NullType,G,R> {typedef R Result;};
template <int N, class T, class G, class R> struct EClos<Set<N,T>,G,R>
{ private:
    typedef typename Move<N,G,NullType>::Result L;
    typedef typename Filter<L,typename Append<T,R>::Result,NullType,NotExist>::Result F;
  public:
    typedef typename EClos<typename Append<T,F>::Result,G,
                           typename Set<N,R>
                          >::Result Result;
};

// Вычисление хода из множества вершин                                                                              *
// T - Состояние
// G - Граф
// R - Результирующее состояние
// a - Символ алфавита
template <class T, class G, class R, int a> struct MoveSet;
template <class G, class R, int a> struct MoveSet<NullType,G,R,a> {typedef R Result;};
template <int N, class T, class G, class R, int a> struct MoveSet<Set<N,T>,G,R,a>
{ typedef typename MoveSet<T,G,typename Join<R,typename Move<N,G,NullType,a>::Result>::Result,a>::Result Result;
};

// Вычисление списка состояний, полученных всеми ходами из вершины                                                  *
// N - Генератор номеров узлов
// K - Генератор номеров финальных узлов
// T - Алфавит
// n - Текущий узел
// S - Текущее состояние (Set)
// G - Граф
// R - Результирующий список расширенных состояний
template <int N, int K, class T, int n, class S, class G, class R> struct MoveList;
template <int N, int K, int n, class S, class G, class R> struct MoveList<N,K,NullType,n,S,G,R> {typedef R Result;};
template <int N, int K, int a, class T, int n, class S, class G, class R> struct MoveList<N,K,Set<a,T>,n,S,G,R>
{ private:
    typedef typename MoveSet<S,G,NullType,a>::Result S0;
    typedef typename EClos<S0,G,NullType>::Result S1;
    enum { N1 = (NotExist<1,S1>::Result)?K:N };
  public:
    typedef typename MoveList<(N==N1)?(N+1):N,
                              (K==N1)?(K+1):K,
                              T,n,S,G,
                              StateListEx<n,N1,a,S1,R> >::Result Result;
};

// Построение алфавита языка по графу NFA (вычислять однократно на верхнем уровне)                                  *
// T - Граф
// R - Результирующий алфавит
template <class T, class R> struct Alf;
template <class R> struct Alf<NullType,R> {typedef R Result;};
template <class T, int S, int D, class R> struct Alf<Edge<T,S,D,0>,R> {typedef typename Alf<T,R>::Result Result;};
template <class T, int S, int D, int a, class R> struct Alf<Edge<T,S,D,a>,R>
{ typedef typename Alf<T, typename AddSet<a,R,NullType>::Result>::Result Result;
};

// Инкремент генератора узлов                                                                                       *
// T - Список состояний (StateListEx)
// R - Результирующее значение генератора
// F - Предикат (Exist, NotExist)
template <class T, int R, template <int,class> class F> struct Incr;
template <int R, template <int,class> class F> struct Incr<NullType,R,F> {enum {Result = R};};
template <int Src, int N, int a, class S, class T, int R, template <int,class> class F> struct Incr<StateListEx<Src,N,a,S,T>,R,F>
{ enum { Result = Incr<T, (F<1,S>::Result)?((N>=R)?(N+1):R):R, F>::Result};
};

// Определение значимого узла                                                                                       *
// N - Узел
// G - Граф
template <int N, class G> struct Important;
template <int N> struct Important<N,NullType> {enum {Result = (N==1)};};
template <int N, class T, int D> struct Important<N,Edge<T,N,D,0> > {enum { Result = Important<N,T>::Result };};
template <int N, class T, int D, int C> struct Important<N,Edge<T,N,D,C> > {enum {Result = true};};
template <int N, class T, int S, int D, int C> struct Important<N,Edge<T,S,D,C> > {enum { Result = Important<N,T>::Result };};

// Оптимизированное построение списка значимых узлов                                                                *
// T - Граф
// R - Результирующий список
template <class T, class R> struct ImportantOpt;
template <class R> struct ImportantOpt<NullType,R> {typedef typename AddSet<1,R,NullType>::Result Result;};
template <class T, int S, int D, class R> struct ImportantOpt<Edge<T,S,D,0>,R>
{ typedef typename ImportantOpt<T,R>::Result Result;
};
template <class T, int S, int D, int C, class R> struct ImportantOpt<Edge<T,S,D,C>,R>
{ typedef typename ImportantOpt<T,typename AddSet<S,R,NullType>::Result>::Result Result;
};

// Сравнение состояний по совокупности значимых узлов
// A - Список узлов (Set)
// B - Список узлов (Set)
// G - Граф
// I - Список значимых узлов (вычислять однократно на верхнем уровне)
template <class A, class B, class G> struct EquEx
{ private:
    typedef typename Filter<A,G,NullType,Important>::Result A1;
    typedef typename Filter<B,G,NullType,Important>::Result B1;
  public:
    enum { Result = Equ<A1,B1>::Result };
};
template <class A, class B, class I> struct EquExOpt
{ private:
    typedef typename Filter<A,I,NullType,Exist>::Result A1;
    typedef typename Filter<B,I,NullType,Exist>::Result B1;
  public:
    enum { Result = Equ<A1,B1>::Result };
};

// Получение списка узлов                                                                                           *
// G - Граф
// R - Результирующий список
template <class T, class R> struct EdgeList;
template <class R> struct EdgeList<NullType,R> {typedef R Result;};
template <class T, int S, int D, int C, class R> struct EdgeList<Edge<T,S,D,C>,R>
{ private:
    typedef typename AddSet<S,R, NullType>::Result R0;
    typedef typename AddSet<D,R0,NullType>::Result R1;
  public:
    typedef typename EdgeList<T,R1>::Result Result;
};

// Проверка вхождения (по равенству состояний)
// T - Контрольный список (StateList)
// S - Искомое состояние (Set)
// I - Список значимых узлов
template <class T, class S, class I> struct ExistS;
template <class S, class I> struct ExistS<NullType,S,I> {enum {Result = false};};
template <int N, class s, class T, class S, class I> struct ExistS<StateList<N,s,T>,S,I>
{ enum { Result = (Equ<s,S>::Result) ? // EquExOpt<s,S,I>::Result
                  true :
                  ExistS<T,S,I>::Result
       };
};

// Отброс ранее найденных узлов
// T - Исходный список (StateListEx)
// С - Контрольный список (StateList)
// I - Список значимых узлов (Set)
// R - Результирующий список (StateListEx)
template <class T, class C, class I, class R> struct FilterT;
template <class C, class I, class R> struct FilterT<NullType,C,I,R> {typedef R Result;};
template <int Src, int Dst, int a, class S, class T, class C, class I, class R> struct FilterT<StateListEx<Src,Dst,a,S,T>,C,I,R>
{ typedef typename If<ExistS<C,S,I>::Result,
                      typename FilterT<T,C,I,R>::Result,
                      typename FilterT<T,C,I,StateListEx<Src,Dst,a,S,R> >::Result
                     >::Result Result;
};

// Формирование результирующего графа
// T - Множество ранее сформированных вершин (StateList)
// a - Символ перехода к искомой вершине
// S - Исходное состояние (Set)
// I - Список значимых узлов
// R - Формируемый граф
template <class T, int Src, int Dst, int a, class S, class I, class R> struct GenImpl;
template <int Src, int Dst, int a, class S, class I, class R> struct GenImpl<NullType,Src,Dst,a,S,I,R> {typedef R Result;};
template <int n, class s, class T, int Src, int Dst, int a, class S, class I, class R> struct GenImpl<StateList<n,s,T>,Src,Dst,a,S,I,R>
{ typedef typename If<Equ<s,S>::Result, // EquExOpt<s,S,I>
                      Edge<R,Src,n,a>,
                      typename GenImpl<T,Src,Dst,a,S,I,R>::Result
                     >::Result Result;
};

// Формирование результирующего графа
// T - Множество новых узлов
// С - Ранее сформированные узлы
// I - Множество значимых узлов
// R - Результирующий граф
template <class T, class C, class I, class R> struct Gen;
template <class C, class I, class R> struct Gen<NullType,C,I,R> {typedef R Result;};
template <int Src, int Dst, int a,class S, class T, class C, class I, class R> struct Gen<StateListEx<Src,Dst,a,S,T>,C,I,R>
{ typedef typename Gen<T,C,I,typename GenImpl<C,Src,Dst,a,S,I,R>::Result>::Result Result;
};

// Шаг преобразования
// N - Генератор номеров результирующих узлов
// K - Генератор номеров финальных узлов
// G - Граф (NFA)
// A - Алфавит (Set)
// I - Список значимых узлов (Set)
// R - Результирующий граф (DFA)
// M - Список помеченных состояний (StateList)
// D - Список непомеченных состояний (StateListEx)
template <int N, int K, class G, class A, class I, class R, class M, class D> struct ConvertImpl;
template <int N, int K, class G, class A, class I, class R, class M> struct ConvertImpl<N,K,G,A,I,R,M,NullType> {typedef R Result;};
template <int N, int K, class G, class A, class I, class R, class M, int Src, int Dst, int a, class S, class D> 
struct ConvertImpl<N,K,G,A,I,R,M,StateListEx<Src,Dst,a,S,D> >
{ private:
    typedef typename MoveList<N,K,A,Dst,S,G,NullType>::Result T;                      // Список ходов из текущего узла             *
    typedef typename StateList<Dst,S,M> M1;                                           // Добавление в список помеченных узлов
    typedef typename Append<D,M1>::Result MD;                                         // Совокупность всех найденных узлов        *
    typedef typename FilterT<T,MD,I,NullType>::Result T1;                             // Отброс ранее найденных
    typedef typename AppendSafe<T1,D>::Result D1;                                     // Добавление новых узлов к непомеченным    *
    typedef typename Gen<T,typename Append<T1,MD>::Result,I,R>::Result R1;            // Обработка T построение графа R1
    enum { N1 = Incr<T1,N,Exist>::Result,                                             // Инкремент счетчика финальных узлов
           K1 = Incr<T1,K,NotExist>::Result                                           // Инкремент счетчика обычных узлов
         };
  public:
    typedef typename ConvertImpl<N1,K1,G,A,I,R1,M1,D1>::Result Result;
};

// Преобразование NFA -> DFA
// G - Граф
// R - Результирующий граф
template <class G, class R> struct Convert
{ private:
    typedef typename Alf<G,NullType>::Result A;
    typedef typename ImportantOpt<G,NullType>::Result I;
  public:
    typedef typename ConvertImpl<1,MAX_FIN_STATE+1,G,A,I,NullType,NullType,
                                 StateListEx<0,0,0,typename EClos<Set<0,NullType>,G,NullType>::Result,NullType> >::Result Result;
};

template <class T>
class DFA: public DFAImpl<typename Convert<typename T::Result,NullType>::Result> {};

};
