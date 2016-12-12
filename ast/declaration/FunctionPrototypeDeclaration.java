package ast.declaration;

import ast.*;
import ast.type.Type;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 12/8/16.
 */
@Value
public class FunctionPrototypeDeclaration implements Declaration {
    private Type returnType;
    private String identifier;
    private AbstractParameterList parameterList;
    @Override
    public String toCode() {
        return returnType.toCode() + " " + identifier + "(" + parameterList.toCode() + ");";
    }

    @Override
    public boolean isCritical() {
        return true;
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
}
