package visitor;

import lombok.Value;
import java.util.*;
import ast.*;
import ast.type.*;
import static ast.Declaration.DeclaredVariable;
import java.util.Map;

@Value
public class ASTGeneralVisitor {

   private TypeScope globalScope;

   public ASTGeneralVisitor(Program p) {

      TypeScope globalScope = new TypeScope();
      globalScope.add();

      Map<String, Type> types = new HashMap<>();
      putDeclarations(p.getDeclaration(), types);

      functions = p.getFunctions();
      for (Function f : functions) {
         String functionId = f.getIdentifier();
         Type functionReturnType = f.getReturnType();
         globalScope.put(functionId, functionReturnType);
         Map<String, Type> functionScope = getDeclaration(stm);
      }
   }

   private Map<String, Type> getDeclarations(CompoundStatement stm) {
      Collection<BlockItem> blockItems = stm.getBlockItems();
      Collection<Declaration> declarations = blockItems.stream().filter(blk -> blk instanceof Declaration).collect(Collections(Collectors.toList()));
         getFunctionDeclarations(declarations);
      Map<String, Type> types = new HashMap<>();
      for (Declaration d : declarations) {
         putDeclarations(d, types);
      }
      return types;
   }

   private void putDeclarations(Declaration d, Map<String, Type> types) {
      declaredVariables = d.getDeclaredVariables();
      for (DeclaredVariable var : declaredVariables) {
         String id = var.getIdentifier();
         Type type = var.getType();
         types.put(id, type);
      }
   }
}
