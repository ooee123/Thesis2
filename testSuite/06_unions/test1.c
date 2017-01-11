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

int main() {
   BStruct b;
   b.b.cstruct.blahblah = 2;
   b.b.cstruct.yesyes = '2';
   return b.b.cstruct.blahblah;
}
