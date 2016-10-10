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
      this(null);
   }

   public TypeScope(TypeScope parent) {
      this(new HashMap<>(), parent);
   }

   public void put(String id, Type type) {
      types.put(id, type);
   }

   public void putAll(Map<? extends String, ? extends Type> moreTypes) {
      types.putAll(moreTypes);
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
