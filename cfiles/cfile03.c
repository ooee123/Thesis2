int main() {
   const char* start = "On the %s day of Christmas my true love sent to me";
   const char* verses[] = {"a Partridge in a Pear Tree.", "Two Turtle Doves", "Three French Hens", "Four Calling Birds", "Five Gold Rings", "Six Geese a-Laying", "Seven Swans a-Swimming", "Eight Maids a-Milking", "Nine Ladies Dancing", "Ten Lords a-Leaping", "Eleven Pipers Piping", "Twelve Drummers Drumming"};
   const char* days[] = {"First", "Second", "Third", "Fourth", "Fifth", "Sixth", "Seventh", "Eighth", "Ninth", "Tenth", "Eleventh", "Twelveth"};
   const int versesCount = 12;
   int i, j;
   for (i = 0; i < versesCount; i++) {
      printf(start, days[i]);
      printf("\n");
      for (j = i; j >= 0; j--) {
         if (i > 0 && j == 0) {
            printf("and ");
         }
         printf(verses[j]);
         printf("\n");
      }
   }
   return 0;
}
