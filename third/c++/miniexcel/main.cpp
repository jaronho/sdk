#include <stdio.h>
#include <stdlib.h>

#include "mexcel.h"
using namespace miniexcel;

#define SAVEPATH "c:\\temp\\a.xls"
//#define SAVEPATH "/home/andy/tmp/a.xls"

int main (int argc, char **args)
{
  
  FILE *f = fopen (SAVEPATH, "wb");

  CMiniExcel miniexcel;
  
  miniexcel(0,0) = "Item1:";
  miniexcel(1,0) = "Item2:";
  miniexcel(2,0) = "Sum = ";
  miniexcel(2,0).setBorder(BORDER_LEFT | BORDER_TOP | BORDER_BOTTOM);
  miniexcel(2,0).setAlignament(ALIGN_CENTER);

	
  miniexcel(0,1) = 10;
  miniexcel(1,1) = 20;
  miniexcel(2,1) = (double)miniexcel(0,1) + (double)miniexcel(1,1);
  miniexcel(2,1).setBorder(BORDER_RIGHT | BORDER_TOP | BORDER_BOTTOM);
	
  miniexcel.Write(f);	

  return 0;
}
