package ast;

import ast.type.Type;
import lombok.AllArgsConstructor;
import lombok.NonNull;
import lombok.Value;

import java.util.List;
import java.util.Map;

/**
 * Created by ooee on 9/24/16.
 */
@Value
public class Declaration implements BlockItem {

    @Value
    @AllArgsConstructor
    public static class DeclaredVariable {
        @NonNull private Type type;
        @NonNull private String identifier;
        private AssignmentExpression initializer;

        public DeclaredVariable(Type type, String identifier) {
            this.type = type;
            this.identifier = identifier;
            this.initializer = null;
        }
    }

    private List<DeclaredVariable> declaredVariables;
}
