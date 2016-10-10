package ast;

import lombok.Value;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class ExclusiveBitwiseOrExpressionImpl implements ExclusiveBitwiseOrExpression {
    public static final String BITWISE_OR = "^";

    private Collection<BitwiseAndExpression> bitwiseAndExpressions;

    @Override
    public String toCode() {
        List<String> codes = bitwiseAndExpressions.stream().map(exp -> exp.toCode()).collect(Collectors.toList());
        return String.join(BITWISE_OR, codes);
    }

    @Override
    public Set<String> getLValues() {
        return Collections.emptySet();
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        for (BitwiseAndExpression bitwiseAndExpression : bitwiseAndExpressions) {
            variables.addAll(bitwiseAndExpression.getVariables());
        }
        return variables;
    }
}
