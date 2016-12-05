package ast.declaration;

import ast.Expression;
import ast.type.EnumType;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 12/4/16.
 */
@Value
public class EnumDefinition implements TypeDefinition {
    private EnumType enumType;

    @Override
    public String toCode() {
        return "struct " + enumType.toCode();
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
    public void visitAllExpressions(Visitor<Expression> visitor) {

    }

    @Override
    public String definition() {
        return enumType.expandedStructUnion();

    }
}
