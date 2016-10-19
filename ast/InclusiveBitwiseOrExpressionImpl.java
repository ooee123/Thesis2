package ast;

import lombok.Value;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class InclusiveBitwiseOrExpressionImpl implements InclusiveBitwiseOrExpression {
    public static final String INCLUSIVE_BITWISE_OR = "|";

    private Collection<ExclusiveBitwiseOrExpression> exclusiveBitwiseOrExpressions;

    @Override
    public String toCode() {
        List<String> codes = exclusiveBitwiseOrExpressions.stream().map(exp -> exp.toCode()).collect(Collectors.toList());
        return String.join(INCLUSIVE_BITWISE_OR, codes);
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return multiGetGuaranteedChangedVariables(exclusiveBitwiseOrExpressions.toArray(new Expression[0]));
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        for (ExclusiveBitwiseOrExpression exclusiveBitwiseOrExpression : exclusiveBitwiseOrExpressions) {
            variables.addAll(exclusiveBitwiseOrExpression.getVariables());
        }
        return variables;
    }

    @Override
    public Set<String> getDependentVariables() {
        return multiGetDependentVariables(exclusiveBitwiseOrExpressions.toArray(new Expression[0]));
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGetInvocations(exclusiveBitwiseOrExpressions.toArray(new Expression[0]));
    }
}
