#include <map>
#include <string>
#include <string.h>
#include <algorithm>
#include <cctype>
#include <ptree.h>
#include <token-tree-map.h>

using namespace std;

map<string, int> name2id;
map<int, string> id2name;

#ifdef JAVA
string identifierTypeName = "ID_TK";
#else
#ifdef PHP
string identifierTypeName = "T_VARIABLE";
#else
string identifierTypeName = "IDENTIFIER";
#endif
#endif

void printTree(Tree *t) {
   for (int i = 0; i < t->children.size(); i++) {
      if (t->children[i]->isTerminal()) {
         Terminal *term = static_cast<Terminal*>(t->children[i]);
         printf("%s ", term->value->c_str());
      } else {
         if (t->children[i]->terminal_number >= 0) {
            printTree(t->children[i]);
         }
      }
   }
}

int main(int argc, char* argv[]) {
   id_init();
   if (argc < 4) {
      fprintf(stderr, "usage: %s <file_name> <start> <end>\n", argv[0]);
      return -1;
   }
   char *filename = argv[1];
   int start = atoi(argv[2]);
   int end = atoi(argv[3]);
   ParseTree *t = parseFile(filename);
   Tree *tree = t->tokenRange2Tree(start, end);
	printTree(tree);
   return 0;
}

