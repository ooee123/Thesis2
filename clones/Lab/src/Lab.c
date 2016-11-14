#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DIM1 6
#define DIM2 8
#define NAME_LEN 20

#define qsort NOT_ALLOWED


typedef struct Name {
   char first[NAME_LEN+1];
   char last[NAME_LEN+1];
} Name;

// Support functions NameLess, NameGreater, DblLess, and StrLess
int NameLess(void *name1, void *name2)
{
   int i;
   if (i = strcmp(((Name)name1)->last, ((Name)name2)->last)
   {
   }
   else
   {
      i = strcmp(((Name)name1)->first, ((Name)name2)->first;
   }
   return i;
}
   
int NameGreater(void *name1, void *name2)
{
   int i;
   if (i = strcmp(((Name)name2)->last, ((Name)name1)->last)
   {
   }
   else
   {
      i = strcmp(((Name)name2)->first, ((Name)name1)->first;
   }
   return i;
}

int DblLess(void *dbl1, void *dbl2)
{
   return dbl1 < dbl2;
}

int StrLess(void *str1, void *str2)
{
   return strcmp(str1, str2) < 0;
}
   

// Function GenInsertSort
void GenInsertSort(void *vals, int dimension, int bytes, int (*rule)(void*, void*)) {
   int ndx, insNdx, toInsert;

   for (ndx = 1; ndx < size; ndx++) {
      toInsert = vals[insNdx = ndx];
      while (insNdx > 0 && (*rule)(toInsert, vals[insNdx-1])) {
         vals[insNdx] = vals[insNdx-1];
         insNdx--;
      }
      vals[insNdx] = toInsert;
   }
}




void main() {
   Name names[DIM1] = {{"John", "Smith"}, {"Jane", "Smith"}, {"Bill", "Jones"},
      {"Sue", "Johnson"}, {"Susan", "Johnson"}, {"Jim", "Taylor"}
   };
   double vals[DIM2] = {1.1, -2.2, 3.3, -4.2, 5.5, -6.6, 7.7, -8.8};
   char *words[DIM2]
    = {"alpha", "beta", "gamma", "delta", "epsilon", "pi", "phi", "omega"};
   int i;
   
   GenInsertSort(names, DIM1, sizeof(Name), NameLess);
   for (i = 0; i < DIM1; i++)
      printf("%s %s\n", names[i].first, names[i].last);
	  
   GenInsertSort(names, DIM1, sizeof(Name), NameGreater);   
   for (i = 0; i < DIM1; i++)
      printf("%s %s\n", names[i].first, names[i].last);

   GenInsertSort(vals, DIM2, sizeof(double), DblLess);
   for (i = 0; i < DIM2; i++)
      printf("%f ", vals[i]);
   printf("\n");
   
   GenInsertSort(words, DIM2, sizeof(char *), StrLess);
   for (i = 0; i < DIM2; i++)
      printf("%s ", words[i]);
   printf("\n");
}
