%module tinysplinego

//********************************************************
//*                                                      *
//* BSpline (Go)                                         *
//*                                                      *
//********************************************************
%ignore ts::BSpline::operator=;
%ignore ts::BSpline::operator();
%ignore tinyspline::Domain::operator=;

//********************************************************
//*                                                      *
//* DeBoorNet (Go)                                       *
//*                                                      *
//********************************************************
%ignore ts::DeBoorNet::operator=;

//********************************************************
//*                                                      *
//* SWIG base file                                       *
//*                                                      *
//********************************************************
%include "tinyspline.i"