package ast.declaration;

import ast.BaseElement;
import ast.declaration.Declaration;
import ast.expression.AssignmentExpression;
import ast.expression.Expression;
import ast.expression.impl.PrimaryExpressionIdentifier;
import ast.type.ActualType;
import ast.type.PointerType;
import ast.type.Type;
import lombok.AllArgsConstructor;
import lombok.NonNull;
import lombok.Value;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 9/24/16.
 */
@Value
@AllArgsConstructor
public class VariableDeclaration implements Declaration {

    @Value
    @AllArgsConstructor
    public static class DeclaredVariable implements BaseElement {
        @NonNull private Type type;
        @NonNull private PrimaryExpressionIdentifier identifier;
        private AssignmentExpression initializer;
        private boolean array;
        private AssignmentExpression initialSize;

        public DeclaredVariable(Type type, PrimaryExpressionIdentifier identifier, AssignmentExpression assignmentExpression) {
            this(type, identifier, assignmentExpression, false, null);
        }

        public DeclaredVariable(Type type, PrimaryExpressionIdentifier identifier) {
            this(type, identifier, null);
        }

        @Override
        public String toCode(boolean showOriginalLine) {
            throw new UnsupportedOperationException();
        }
    }

    private List<DeclaredVariable> declaredVariables;
    private String originalLine;
/*
    public VariableDeclaration(List<DeclaredVariable> declaredVariables) {
        this(declaredVariables, toCommentTip(declaredVariables));
    }
*/
    @Override
    public Set<String> getDependantVariables() {
        Set<String> declaredVariables = new HashSet<>();
        Set<String> dependentVariables = new HashSet<>();
        Set<String> guaranteedChangedVariables = new HashSet<>();
        for (DeclaredVariable declaredVariable : this.declaredVariables) {
            declaredVariables.add(declaredVariable.getIdentifier().getIdentifier());
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
                guaranteedChangedVariables.add(declaredVariable.getIdentifier().getIdentifier());
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
                guaranteedChangedVariables.add(declaredVariable.getIdentifier().getIdentifier());
                guaranteedChangedVariables.addAll(declaredVariable.getInitializer().getGuaranteedChangedVariables());
            }
        }
        return guaranteedChangedVariables;
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        if (showOriginalLine) {
            return toCommentTip(this.declaredVariables) + " /* " + originalLine + " */";
        } else {
            return toCommentTip(this.declaredVariables);
        }
    }

    private static String toCommentTip(List<DeclaredVariable> declaredVariables) {
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
            builder.append(variable.identifier.toCode());
            if (variable.isArray()) {
                builder.append("[");
                if (variable.getInitialSize() != null) {
                    builder.append(variable.getInitialSize().toCode());
                }
                builder.append("]");
            }
            if (variable.initializer != null) {
                builder.append(" = " + variable.initializer.toCode());
            }
            declaredVariableCode.add(builder.toString());
        }
        String thusFar = actualType.toCode() + " " + String.join(", ", declaredVariableCode) + ";";
        return thusFar;
    }

    @Override
    public boolean isCritical() {
        return true;
    }

    @Override
    public void visitOwnedExpressions(Visitor<Expression> visitor) {
        for (DeclaredVariable declaredVariable : declaredVariables) {
            if (declaredVariable.getInitializer() != null) {
                visitor.visit(declaredVariable.getInitializer());
            }
        }
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        visitOwnedExpressions(visitor);
    }
}
