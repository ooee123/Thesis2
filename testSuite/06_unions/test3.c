typedef union {
   int number;
   double decimal;
   char letter;
} AUnion;

typedef struct {
   int number;
   double decimal;
   char letter;
   AUnion un;
} AStruct;

typedef struct {
   int blahblah;
   char yesyes;
} CStruct;

typedef union {
   char *string;
   int number;
   CStruct cstruct;
} Bunion;

typedef struct {
   Bunion b;
   AStruct a;
} BStruct;

Bunion example_union;

int main() {
   BStruct b;
   b.b.cstruct.blahblah = 2;
   b.b = example_union;
   return b.b.cstruct.blahblah;
}
