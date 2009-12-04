/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


// triple.h

// Template class for struct that holds three items

#ifndef __TRIPLE__MULBERRY__
#define __TRIPLE__MULBERRY__

template <class T1, class T2, class T3>
struct ctriple {

    typedef T1    first_type;
    typedef T2    second_type;
    typedef T3    third_type;

    T1 first;
    T2 second;
    T3 third;

    ctriple () : first (T1 ()), second (T2 ()), third (T3 ()) {}
    ctriple (const T1& a, const T2& b, const T3& c)
    : first (a), second (b), third (c) {}

#ifdef MSIPL_MEMBER_TEMPLATE
    template<class U, class V, class W>
    ctriple (const ctriple<U, V, W> &p): first(p.first), second (p.second), third (p.third) {}
#else
    ctriple (const ctriple &p): first(p.first), second (p.second), third (p.third) {}
#endif

};

template <class T1, class T2, class T3>
inline bool
operator== (const ctriple<T1, T2, T3>& x, const ctriple<T1, T2, T3>& y)
{ 
    return x.first == y.first && x.second == y.second && x.third == y.third; 
}

template <class T1, class T2, class T3>
inline bool
operator< (const ctriple<T1, T2, T3>& x, const ctriple<T1, T2, T3>& y)
{ 
    return x.first < y.first ||
    		(! (y.first < x.first) && x.second < y.second) ||
    		(! (y.first < x.first) && ! (y.second < x.second) && x.third < y.third); 
}

template <class T1, class T2, class T3>
inline ctriple<T1, T2, T3>
make_ctriple (const T1& x, const T2& y, const T3& z)
{
    return ctriple<T1, T2, T3> (x, y, z);
}

#endif
