package ast;

import lombok.NonNull;
import lombok.Value;

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
}
