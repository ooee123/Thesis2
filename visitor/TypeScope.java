package visitor;

import ast.type.Type;
import lombok.AllArgsConstructor;
import lombok.Value;

import java.util.HashMap;
import java.util.Map;

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

   private void flatten(Map<String, Type> soFar) {
      for (Map.Entry<String, Type> stringTypeEntry : types.entrySet()) {
         if (!soFar.containsKey(stringTypeEntry.getKey())) {
            soFar.put(stringTypeEntry.getKey(), stringTypeEntry.getValue());
         }
      }
      if (parent != null) {
         parent.flatten(soFar);
      }
   }

   public Map<String, Type> flatten() {
      Map<String, Type> types = new HashMap<>();
      types.putAll(this.types);
      if (parent != null) {
         parent.flatten(types);
      }
      return types;
   }
}
