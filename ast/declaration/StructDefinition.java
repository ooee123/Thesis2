package ast.declaration;

import ast.expression.Expression;
import ast.type.StructUnionType;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 11/5/16.
 */
@Value
public class StructDefinition implements TypeDefinition {
    private StructUnionType structUnionType;

    @Override
    public String definition() {
        return structUnionType.expandedStructUnion();
    }

    @Override
    public String toCode() {
        return structUnionType.expandedStructUnion() + ";";
    }

    @Override
    public boolean isCritical() {
        return false;
    }

    @Override
    public Set<String> getDependantVariables() {
        return null;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return null;
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        return null;
    }

    @Override
    public void visitOwnedExpressions(Visitor<Expression> visitor) {
        return;
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        visitOwnedExpressions(visitor);
    }
}
