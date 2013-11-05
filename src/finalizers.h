// -*- mode: C++; c-indent-level: 2; c-basic-offset: 2; tab-width: 8 -*-
///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2011  Whit Armstrong                                    //
//                                                                       //
// This program is free software: you can redistribute it and/or modify  //
// it under the terms of the GNU General Public License as published by  //
// the Free Software Foundation, either version 3 of the License, or     //
// (at your option) any later version.                                   //
//                                                                       //
// This program is distributed in the hope that it will be useful,       //
// but WITHOUT ANY WARRANTY; without even the implied warranty of        //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         //
// GNU General Public License for more details.                          //
//                                                                       //
// You should have received a copy of the GNU General Public License     //
// along with this program.  If not, see <http://www.gnu.org/licenses/>. //
///////////////////////////////////////////////////////////////////////////

#ifndef FINALIZERS_H
#define FINALIZERS_H

typedef void(*finalizerT)(SEXP);

template<typename T>
void finalizeSEXP(SEXP p_) {
  if(p_ == R_NilValue) {
    return;
  }

  T* p = reinterpret_cast<T*>(R_ExternalPtrAddr(p_));
  if(p) {
    delete p;
    R_ClearExternalPtr(p_);
  }
}

/*
  template<typename T>
  void addExternalPoniter(SEXP x, T* p, finalizerT finalizer, const char* atty_name, const char* pname) {
  SEXP p_;
  PROTECT(p_ = R_MakeExternalPtr(reinterpret_cast<void*>(p),Rf_install(pname),R_NilValue));
  R_RegisterCFinalizerEx(p_, finalizer, TRUE);
  Rf_setAttrib(x, Rf_install(atty_name), p_);
  UNPROTECT(1);
  }
*/

template<typename T>
SEXP createExternalPointer(T* p, finalizerT finalizer, const char* pname) {
  SEXP p_;
  PROTECT(p_ = R_MakeExternalPtr(reinterpret_cast<void*>(p),Rf_install(pname),R_NilValue));
  R_RegisterCFinalizerEx(p_, finalizer, TRUE);
  UNPROTECT(1);
  return p_;
}

#endif // FINALIZERS_H
