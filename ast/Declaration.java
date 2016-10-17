package ast;

import ast.type.ActualType;
import ast.type.PointerType;
import ast.type.Type;
import lombok.AllArgsConstructor;
import lombok.NonNull;
import lombok.Value;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by ooee on 9/24/16.
 */
@Value
public class Declaration implements BlockItem {

    @Value
    @AllArgsConstructor
    public static class DeclaredVariable implements BaseElement {
        @NonNull private Type type;
        @NonNull private String identifier;
        private AssignmentExpression initializer;

        public DeclaredVariable(Type type, String identifier) {
            this.type = type;
            this.identifier = identifier;
            this.initializer = null;
        }

        @Override
        public String toCode() {
            return null;
        }
    }

    private List<DeclaredVariable> declaredVariables;

    @Override
    public String toCode() {
        DeclaredVariable declaredVariable = declaredVariables.stream().findAny().get();
        ActualType actualType;
        if (declaredVariable.type instanceof PointerType) {
            actualType = ((PointerType) declaredVariable.type).getActualType();
        } else {
            actualType = (ActualType)declaredVariable.type;
        }


        List<String> declaredVariableCode = new ArrayList<>();
        for (DeclaredVariable variable : declaredVariables) {
            StringBuilder builder = new StringBuilder();
            Type variableType = variable.type;
            if (variableType instanceof PointerType) {
                for (int i = 0; i < ((PointerType) variableType).getPointerLevel(); i++) {
                    builder.append("*");
                }
            }
            builder.append(variable.identifier);
            if (variable.initializer != null) {
                builder.append(" = " + variable.initializer.toCode());
            }
            declaredVariableCode.add(builder.toString());
        }
        return actualType.toCode() + " " + String.join(", ", declaredVariableCode) + ";";
    }

    @Override
    public boolean isCritical() {
        return true;
    }
}
