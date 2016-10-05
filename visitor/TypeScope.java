package visitor;

import lombok.Value;
import lombok.AllArgsConstructor;

import java.util.*;
import ast.*;
import ast.type.Type;

@Value
@AllArgsConstructor
public class TypeScope {
   private Map<String, Type> types;
   private TypeScope parent;

   public TypeScope() {
      types = new HashMap<>();
      parent = null;
   }
}
