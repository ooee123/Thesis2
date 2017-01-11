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
   b.b.string = "hello";
   return b.b.number;
}
