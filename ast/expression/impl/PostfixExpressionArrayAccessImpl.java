package ast.expression.impl;

import ast.expression.Expression;
import ast.expression.PostfixExpression;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionArrayAccessImpl implements PostfixExpression {
    private PostfixExpression postfixExpression;
    private Expression arrayIndex;

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return multiGetGuaranteedChangedVariables(postfixExpression, arrayIndex);
    }

    public String toCode() {
        return postfixExpression.toCode() + "[" + arrayIndex.toCode() + "]";
    }

    @Override
    public Set<String> getVariables() {
        return multiGetVariables(postfixExpression, arrayIndex);
    }

    @Override
    public Set<String> getDependentVariables() {
        return multiGetDependentVariables(postfixExpression, arrayIndex);
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGetInvocations(postfixExpression, arrayIndex);
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(postfixExpression);
        visitor.visit(arrayIndex);
    }
}
