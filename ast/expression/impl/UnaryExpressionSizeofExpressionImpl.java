package ast.expression.impl;

import ast.expression.Expression;
import ast.expression.UnaryExpression;
import ast.expression.impl.PostfixExpressionInvocationImpl;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;


/**
 * Created by ooee on 9/26/16.
 */
@Value
public class UnaryExpressionSizeofExpressionImpl implements UnaryExpression {
    private UnaryExpression unaryExpression;

    @Override
    public Set<String> getDependentVariables() {
        return unaryExpression.getDependentVariables();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return unaryExpression.getInvocations();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return unaryExpression.getGuaranteedChangedVariables();
    }

    public String toCode() {
        return "sizeof " + unaryExpression.toCode();
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
