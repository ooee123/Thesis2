package ast;

import lombok.NonNull;
import lombok.Value;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class ConditionalExpressionImpl implements ConditionalExpression {
    @NonNull private LogicalOrExpression logicalOrExpression;
    private Expression trueExpression;
    private ConditionalExpression falseExpression;

    @Override
    public String toCode() {
        String trueExpressionCode = "", falseExpressionCode = "";

        if (trueExpression != null) {
            trueExpressionCode = trueExpression.toCode();
        }
        if (falseExpression != null) {
            falseExpressionCode = falseExpression.toCode();
        }
        return logicalOrExpression.toCode() + " ? " + trueExpressionCode + " : " + falseExpressionCode;
    }

    @Override
    public Set<String> getLValues() {
        return Collections.emptySet();
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        variables.addAll(logicalOrExpression.getVariables());
        if (trueExpression != null) {
            variables.addAll(trueExpression.getVariables());
        }
        if (falseExpression != null) {
            variables.addAll(falseExpression.getVariables());
        }
        return variables;
    }
}
