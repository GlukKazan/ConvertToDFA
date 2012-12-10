#include "stdafx.h"
#include "NFA.h"
#include "DFA.h"

using namespace RegExp;

int _tmain(int argc, _TCHAR* argv[])
{
    typedef E< Q< D< C<'a'>, C<'b'> > >, C<'a'>, C<'b'>, C<'b'> >::Result G;

    typedef Convert<G,NullType>::Result R;
    R::Dump();

	return 0;
}

