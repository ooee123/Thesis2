package ast.expression.impl;

import ast.expression.Expression;
import ast.expression.PrimaryExpression;
import ast.expression.UnaryExpression;
import ast.type.Type;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by kly04 on 9/26/16.
 */
@Value
public class PrimaryExpressionVariadicArg implements PrimaryExpression {
    private UnaryExpression unaryExpression;
    private Type typeName;

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return unaryExpression.getVariables();
    }

    public String toCode() {
        return "__buildin_va_arg(" + unaryExpression.toCode() + ", " + typeName.toCode() + ")";
    }

    @Override
    public Set<String> getDependentVariables() {
        return unaryExpression.getVariables();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return unaryExpression.getInvocations();
    }

    @Override
    public Set<String> getVariables() {
        return unaryExpression.getVariables();
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(unaryExpression);
    }
}
