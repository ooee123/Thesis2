package ast.declaration;

import ast.*;
import ast.expression.Expression;
import ast.type.Type;
import lombok.AllArgsConstructor;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 12/8/16.
 */
@Value
@AllArgsConstructor
public class FunctionPrototypeDeclaration implements Declaration {
    private Type returnType;
    private String identifier;
    private AbstractParameterList parameterList;

    private String originalLine;

    public FunctionPrototypeDeclaration(Type returnType, String identifier, AbstractParameterList parameterList) {
        this(returnType, identifier, parameterList, toCommentTip(returnType, identifier, parameterList));
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        if (showOriginalLine) {
            return toCommentTip(returnType, identifier, parameterList) + ";" + " /* " + originalLine + " */";
        } else {
            return toCommentTip(returnType, identifier, parameterList) + ";";
        }
    }

    private static String toCommentTip(Type returnType, String identifier, AbstractParameterList parameterList) {
        return returnType.toCode() + " " + identifier + "(" + parameterList.toCode() + ")";
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
    public void visitOwnedExpressions(Visitor<Expression> visitor) {
        return;
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        visitOwnedExpressions(visitor);
    }
}
