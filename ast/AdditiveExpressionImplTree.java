package ast;

import lombok.Value;

import java.util.HashSet;
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
        return additiveExpression.toCode() + additiveOperator.token + "(" + multiplicativeExpression.toCode() + ")";
    }

    @Override
    public Set<String> getDependentVariables() {
        return multiGetDependentVariables(additiveExpression, multiplicativeExpression);
    }

    @Override
    public Set<String> getChangedVariables() {
        return multiGetChangedVariables(additiveExpression, multiplicativeExpression);
    }

    @Override
    public Set<String> getVariables() {
        return multiGetVariables(additiveExpression, multiplicativeExpression);
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGetInvocations(additiveExpression, multiplicativeExpression);
    }
}
