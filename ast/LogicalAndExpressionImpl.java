package ast;

import lombok.Value;

import java.util.*;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class LogicalAndExpressionImpl implements LogicalAndExpression {
    public static final String AND = "&&";

    private Collection<InclusiveBitwiseOrExpression> inclusiveBitwiseOrExpressions;

    @Override
    public String toCode() {
        List<String> codes = collectionToCode(inclusiveBitwiseOrExpressions);
        return String.join(AND, codes);
    }

    @Override
    public Set<String> getChangedVariables() {
        return multiGetChangedVariables(inclusiveBitwiseOrExpressions.toArray(new Expression[0]));
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        for (InclusiveBitwiseOrExpression inclusiveBitwiseOrExpression : inclusiveBitwiseOrExpressions) {
            variables.addAll(inclusiveBitwiseOrExpression.getVariables());
        }
        return variables;
    }

    @Override
    public Set<String> getDependentVariables() {
        return multiGetDependentVariables(inclusiveBitwiseOrExpressions.toArray(new Expression[0]));
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGetInvocations(inclusiveBitwiseOrExpressions.toArray(new Expression[0]));
    }
}
