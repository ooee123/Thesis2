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

   public void put(String id, Type type) {
      types.put(id, type);
   }

   public Type get(String id) {
      if (types.containsKey(id)) {
         return types.get(id);
      } else if (parent != null) {
         return parent.get(id);
      } else {
         return null;
      }
   }
}
