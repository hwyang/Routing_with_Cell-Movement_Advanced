////////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (c) 2018, Iowa State University All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////

#ifndef __FLUTE_H__
#define __FLUTE_H__

#include <ostream>

namespace Flute {

const int FLUTE_ACCURACY = 10; // Default accuracy

typedef int DTYPE;

struct Branch {
  DTYPE x, y; // starting point of the branch
  int n;      // index of neighbor
};

struct Tree {
  int deg;        // degree
  DTYPE length;   // total wirelength
  Branch *branch; // array of tree branches
};

// User-Callable Functions
void readLUT();
void deleteLUT();
DTYPE flute_wl(int d, DTYPE x[], DTYPE y[], int acc);
Tree flute(int d, DTYPE x[], DTYPE y[], int acc);
DTYPE wirelength(Tree t);
void printtree(Tree t, std::ostream &out);
void plottree(Tree t, std::ostream &out);
void write_svg(Tree t, std::ostream &out, double Scale = 1.0);
void free_tree(Tree t);

// Other useful functions
DTYPE flutes_wl_LD(int d, DTYPE xs[], DTYPE ys[], int s[]);
DTYPE flutes_wl_MD(int d, DTYPE xs[], DTYPE ys[], int s[], int acc);
DTYPE flutes_wl_RDP(int d, DTYPE xs[], DTYPE ys[], int s[], int acc);
Tree flutes_LD(int d, DTYPE xs[], DTYPE ys[], int s[]);
Tree flutes_MD(int d, DTYPE xs[], DTYPE ys[], int s[], int acc);
Tree flutes_HD(int d, DTYPE xs[], DTYPE ys[], int s[], int acc);
Tree flutes_RDP(int d, DTYPE xs[], DTYPE ys[], int s[], int acc);

inline DTYPE flutes_wl_LMD(int d, DTYPE xs[], DTYPE ys[], int s[], int acc);
inline DTYPE flutes_wl_ALLD(int d, DTYPE xs[], DTYPE ys[], int s[], int acc);
inline DTYPE flutes_wl(int d, DTYPE xs[], DTYPE ys[], int s[], int acc);
inline Tree flutes_ALLD(int d, DTYPE xs[], DTYPE ys[], int s[], int acc);
inline Tree flutes(int d, DTYPE xs[], DTYPE ys[], int s[], int acc);
inline Tree flutes_LMD(int d, DTYPE xs[], DTYPE ys[], int s[], int acc);

} // namespace Flute
#endif /* __FLUTE_H__ */
