package ast.declaration;

import ast.expression.Expression;
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
    public String toCode(boolean showOriginalLine) {
        return toCode();
    }

    public String toCode() {
        return enumType.toCode() + ";";
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

    @Override
    public String definition() {
        return enumType.toExpandedCode();
    }
}
