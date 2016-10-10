package ast;

import lombok.Value;

import java.util.Collection;
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
    public Set<String> getLValues() {
        Set<String> lValues = new HashSet<>();
        lValues.addAll(additiveExpression.getLValues());
        lValues.addAll(multiplicativeExpression.getLValues());
        return lValues;
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        variables.addAll(additiveExpression.getVariables());
        variables.addAll(multiplicativeExpression.getVariables());
        return variables;
    }
}
