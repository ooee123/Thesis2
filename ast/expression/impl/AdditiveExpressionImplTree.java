package ast.expression.impl;

import ast.expression.AdditiveExpression;
import ast.expression.Expression;
import ast.expression.MultiplicativeExpression;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class AdditiveExpressionImplTree implements AdditiveExpression {
    private AdditiveExpression additiveExpression;
    private AdditiveOperator additiveOperator;
    private MultiplicativeExpression multiplicativeExpression;

    public String toCode() {
        return additiveExpression.toCode() + additiveOperator.token + multiplicativeExpression.toCode();
    }

    @Override
    public Set<String> getDependentVariables() {
        return multiGetDependentVariables(additiveExpression, multiplicativeExpression);
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return multiGetGuaranteedChangedVariables(additiveExpression, multiplicativeExpression);
    }

    @Override
    public Set<String> getVariables() {
        return multiGetVariables(additiveExpression, multiplicativeExpression);
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGetInvocations(additiveExpression, multiplicativeExpression);
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(additiveExpression);
        visitor.visit(multiplicativeExpression);
    }
}
