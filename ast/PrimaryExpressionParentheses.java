package ast;

import lombok.Value;

import java.util.Collections;
import java.util.Set;

/**
 * Created by kly04 on 9/26/16.
 */
@Value
public class PrimaryExpressionParentheses implements PrimaryExpression {
    private Expression expression;

    @Override
    public Set<String> getChangedVariables() {
        return expression.getChangedVariables();
    }

    public String toCode() {
        return "(" + expression.toCode() + " )";
    }

    @Override
    public Set<String> getDependentVariables() {
        return expression.getDependentVariables();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return expression.getInvocations();
    }

    @Override
    public Set<String> getVariables() {
        return expression.getVariables();
    }
}
