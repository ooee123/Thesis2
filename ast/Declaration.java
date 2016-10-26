package ast;

import ast.type.ActualType;
import ast.type.PointerType;
import ast.type.PrimitiveType;
import ast.type.Type;
import lombok.AllArgsConstructor;
import lombok.NonNull;
import lombok.Value;
import visitor.PDGGenerationVisitor;
import visitor.Visitor;

import java.util.*;

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
            throw new UnsupportedOperationException();
        }
    }

    private List<DeclaredVariable> declaredVariables;

    @Override
    public Set<String> getDependantVariables() {
        Set<String> declaredVariables = new HashSet<>();
        Set<String> dependentVariables = new HashSet<>();
        Set<String> guaranteedChangedVariables = new HashSet<>();
        for (DeclaredVariable declaredVariable : this.declaredVariables) {
            declaredVariables.add(declaredVariable.getIdentifier());
            if (declaredVariable.getInitializer() != null) {
                Set<String> requiredVars = declaredVariable.getInitializer().getDependentVariables();
                requiredVars.removeAll(declaredVariables);
                dependentVariables.addAll(requiredVars);
            }
        }
        return dependentVariables;
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        Set<String> guaranteedChangedVariables = new HashSet<>();
        for (DeclaredVariable declaredVariable : declaredVariables) {
            if (declaredVariable.getInitializer() != null) {
                guaranteedChangedVariables.add(declaredVariable.getIdentifier());
                guaranteedChangedVariables.addAll(declaredVariable.getInitializer().getGuaranteedChangedVariables());
            }
        }
        return guaranteedChangedVariables;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> guaranteedChangedVariables = new HashSet<>();
        for (DeclaredVariable declaredVariable : declaredVariables) {
            if (declaredVariable.getInitializer() != null) {
                guaranteedChangedVariables.add(declaredVariable.getIdentifier());
                guaranteedChangedVariables.addAll(declaredVariable.getInitializer().getGuaranteedChangedVariables());
            }
        }
        return guaranteedChangedVariables;
    }

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

    @Override
    public <T> Collection<T> visitAllExpressions(Visitor<T, Expression> visitor) {
        Collection<T> collection = new ArrayList<>();
        for (DeclaredVariable declaredVariable : declaredVariables) {
            if (declaredVariable.getInitializer() != null) {
                collection.addAll(visitor.visit(declaredVariable.getInitializer()));
            }
        }
        return collection;
    }

    @Override
    public int pointValue() {
        int points = 0;
        for (DeclaredVariable declaredVariable : declaredVariables) {
            if (declaredVariable.getType() instanceof PrimitiveType) {
                points += ((PrimitiveType) declaredVariable.getType()).getTypeName().length();
            } else {
                points += 1;
            }
            if (declaredVariable.getInitializer() != null) {
                points += declaredVariable.getInitializer().pointValue();
            }
        }
        points *= declaredVariables.size();
        return points;
    }
}
