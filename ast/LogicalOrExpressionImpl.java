package ast;

import lombok.Value;

import java.util.*;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class LogicalOrExpressionImpl implements LogicalOrExpression {
    public static final String OR = "||";

    private Collection<LogicalAndExpression> logicalAndExpressions;

    @Override
    public String toCode() {
        List<String> codes = collectionToCode(logicalAndExpressions);
        return String.join(OR, codes);
    }

    @Override
    public Set<String> getLValues() {
        return Collections.emptySet();
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        for (LogicalAndExpression logicalAndExpression : logicalAndExpressions) {
            variables.addAll(logicalAndExpression.getVariables());
        }
        return variables;
    }
}
